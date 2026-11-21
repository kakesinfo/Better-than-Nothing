/*
 *	Better than Nothing
 *	Copyright (C) 2026 kakesinfo
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 * 	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "Terrain/TerrainSystem.hpp"

#include "Terrain/Noise.hpp"
#include <Common/Logger.hpp>
#include <Common/RenderSettings.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Scene/Scene.hpp>
#include <Scene/SceneObject_Components.hpp>
#include <cmath>
#include <vector>

namespace BetterThanNothing
{
	TerrainSystem::TerrainSystem(TerrainConfig config) : m_generator(std::move(config)) {}

	SceneObject TerrainSystem::BuildTerrainAndWater(Scene* scene)
	{
		if (!scene || !scene->GetResourceManager())
			return {};

		ResourceManager*	 resMgr = scene->GetResourceManager();
		const TerrainConfig& cfg = m_generator.GetConfig();

		// Preload all 5 Ground PBR packs in parallel on CPU worker threads, downscaled to cfg.textureMaxDimension
		std::vector<std::string> texPaths;
		texPaths.reserve(15);
		for (const auto& layer : cfg.layerTextures)
		{
			if (!layer.albedoPath.empty())
				texPaths.push_back(layer.albedoPath);
			if (!layer.normalPath.empty())
				texPaths.push_back(layer.normalPath);
			if (!layer.roughnessPath.empty())
				texPaths.push_back(layer.roughnessPath);
		}
		resMgr->PreloadTexturesParallel(texPaths, cfg.textureMaxDimension);

		TerrainMaterialCreateInfo terrainMatInfo{};
		terrainMatInfo.name = "LandscapeTerrainMaterial";
		for (size_t i = 0; i < 5; ++i)
		{
			const auto& l = cfg.layerTextures[i];
			if (!l.albedoPath.empty())
				terrainMatInfo.layers[i].albedoId = resMgr->LoadTexture(l.albedoPath, cfg.textureMaxDimension);
			if (!l.normalPath.empty())
				terrainMatInfo.layers[i].normalId = resMgr->LoadTexture(l.normalPath, cfg.textureMaxDimension);
			if (!l.roughnessPath.empty())
				terrainMatInfo.layers[i].roughnessId = resMgr->LoadTexture(l.roughnessPath, cfg.textureMaxDimension);
		}
		const ResourceId terrainMaterialId = resMgr->CreateTerrainMaterial(terrainMatInfo);

		RenderSettings::Get().waterLevel = cfg.waterLevel;

		SceneObject landscapeRoot = scene->CreateObject("Landscape");
		SceneObject terrainRoot = scene->CreateObject("TerrainChunks");
		SceneObject waterRoot = scene->CreateObject("WaterChunks");
		terrainRoot.SetParent(landscapeRoot);
		waterRoot.SetParent(landscapeRoot);

		m_terrainChunkCount = 0;
		m_waterChunkCount = 0;

		const float originX = m_generator.GetWorldOriginX();
		const float originZ = m_generator.GetWorldOriginZ();

		for (int32_t cz = 0; cz < cfg.chunksZ; ++cz)
		{
			for (int32_t cx = 0; cx < cfg.chunksX; ++cx)
			{
				const float chunkWorldX = originX + static_cast<float>(cx) * cfg.chunkSize;
				const float chunkWorldZ = originZ + static_cast<float>(cz) * cfg.chunkSize;

				// Terrain chunk
				const MeshData terrainMesh = m_generator.GenerateChunkMesh(cx, cz);
				if (!terrainMesh.vertices.empty() && !terrainMesh.indices.empty())
				{
					const ResourceId meshId = resMgr->CreateMesh(terrainMesh);
					SceneObject		 chunkObj = scene->CreateObject("Terrain_" + std::to_string(cx) + "_" + std::to_string(cz));
					chunkObj.SetParent(terrainRoot);
					chunkObj.GetTransform().SetPosition(chunkWorldX, 0.0f, chunkWorldZ);

					auto& meshComp = chunkObj.AddComponent<MeshComponent>();
					meshComp.meshId = meshId;
					meshComp.materialId = terrainMaterialId;

					auto& chunkComp = chunkObj.AddComponent<TerrainChunkComponent>();
					chunkComp.chunkX = cx;
					chunkComp.chunkZ = cz;
					++m_terrainChunkCount;
				}

				// Water chunk (only spawned where terrain drops near/below waterLevel)
				const MeshData waterMesh = m_generator.GenerateWaterChunkMesh(cx, cz);
				if (!waterMesh.vertices.empty() && !waterMesh.indices.empty())
				{
					const ResourceId waterMeshId = resMgr->CreateMesh(waterMesh);
					SceneObject		 waterObj = scene->CreateObject("Water_" + std::to_string(cx) + "_" + std::to_string(cz));
					waterObj.SetParent(waterRoot);
					waterObj.GetTransform().SetPosition(chunkWorldX, 0.0f, chunkWorldZ);

					auto& meshComp = waterObj.AddComponent<MeshComponent>();
					meshComp.meshId = waterMeshId;

					auto& waterComp = waterObj.AddComponent<WaterChunkComponent>();
					waterComp.chunkX = cx;
					waterComp.chunkZ = cz;
					waterComp.waterLevel = cfg.waterLevel;
					++m_waterChunkCount;
				}
			}
		}

		Logger::Log<SUCCESS>(
			"TerrainSystem: built {} terrain chunks ({}x{} m) and {} water chunks (water Y={:.2f}m)",
			m_terrainChunkCount,
			static_cast<int>(m_generator.GetWorldWidth()),
			static_cast<int>(m_generator.GetWorldDepth()),
			m_waterChunkCount,
			cfg.waterLevel
		);

		return landscapeRoot;
	}

	SceneObject TerrainSystem::ScatterTrees(Scene* scene, const TreeScatterConfig& scatterConfig)
	{
		if (!scene)
			return {};

		const float minX = m_generator.GetWorldOriginX() + scatterConfig.worldMargin;
		const float maxX = -m_generator.GetWorldOriginX() - scatterConfig.worldMargin;
		const float minZ = m_generator.GetWorldOriginZ() + scatterConfig.worldMargin;
		const float maxZ = -m_generator.GetWorldOriginZ() - scatterConfig.worldMargin;
		const float minSpacingSq = scatterConfig.minSpacing * scatterConfig.minSpacing;
		const float minHeight = m_generator.GetConfig().waterLevel + scatterConfig.minHeightAboveWater;

		SceneObject forestRoot = scene->CreateObject("ElmForest");
		TerrainNoise::SeededRng rng(scatterConfig.seed);

		struct PlacedTree
		{
			float x = 0.0f;
			float z = 0.0f;
		};
		std::vector<PlacedTree> placed;
		placed.reserve(scatterConfig.targetCount);

		const uint32_t maxAttempts = scatterConfig.targetCount * 28u;
		m_treeCount = 0;

		for (uint32_t attempt = 0; attempt < maxAttempts && m_treeCount < scatterConfig.targetCount; ++attempt)
		{
			const float x = rng.Range(minX, maxX);
			const float z = rng.Range(minZ, maxZ);

			// Keep Sponza courtyard and immediate exterior clear
			if (x >= scatterConfig.excludeMinX && x <= scatterConfig.excludeMaxX &&
				z >= scatterConfig.excludeMinZ && z <= scatterConfig.excludeMaxZ)
				continue;

			const float y = m_generator.SampleHeight(x, z);
			if (y < minHeight)
				continue;

			const float slope = m_generator.SampleSlope(x, z);
			if (slope > scatterConfig.maxSlope)
				continue;

			bool tooClose = false;
			for (const auto& p : placed)
			{
				const float dx = x - p.x;
				const float dz = z - p.z;
				if (dx * dx + dz * dz < minSpacingSq)
				{
					tooClose = true;
					break;
				}
			}
			if (tooClose)
				continue;

			placed.push_back({x, z});

			SceneObject tree = scene->LoadModel(scatterConfig.modelPath, true);
			tree.SetParent(forestRoot);

			const float scale = rng.Range(scatterConfig.minScale, scatterConfig.maxScale);
			const float yawDeg = rng.Range(0.0f, 360.0f);
			const float tiltX = rng.Range(-3.0f, 3.0f);
			const float tiltZ = rng.Range(-3.0f, 3.0f);

			// Sink trunk base ~0.25m into slope so roots never float on hillsides
			tree.GetTransform().SetPosition(x, y - 0.25f, z);
			tree.GetTransform().SetEulerDegrees(Vector3(tiltX, yawDeg, tiltZ));
			tree.GetTransform().SetScale(scale);
			++m_treeCount;
		}

		Logger::Log<SUCCESS>("TerrainSystem: scattered {} elm trees ({})", m_treeCount, scatterConfig.modelPath);
		return forestRoot;
	}
}
