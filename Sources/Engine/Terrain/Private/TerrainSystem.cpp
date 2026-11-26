/*
 *	Better than Nothing
 *	Copyright (C) 2026 kakesinfo
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <Common/AssetPath.hpp>
#include <Common/Logger.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Scene/Scene.hpp>
#include <Scene/SceneObject.hpp>
#include <Scene/SceneObject_Components.hpp>
#include <Terrain/Noise.hpp>
#include <Terrain/TerrainSystem.hpp>
#include <filesystem>

namespace BetterThanNothing
{
	std::array<std::string, 5> TerrainSystem::s_textureOverrides{};

	TerrainSystem::TerrainSystem(const TerrainConfig& config)
		: m_config(config), m_generator(config)
	{
	}

	void TerrainSystem::mapTexture(const TerrainTextureType type, const std::string& texturePath)
	{
		const auto idx = static_cast<size_t>(type);
		if (idx < s_textureOverrides.size())
			s_textureOverrides[idx] = texturePath;
	}

	const std::array<std::string, 5>& TerrainSystem::GetTextureOverrides()
	{
		return s_textureOverrides;
	}

	std::vector<Vector3> TerrainSystem::ComputeTreePositions(const TreeScatterConfig& config) const
	{
		std::vector<Vector3> positions;
		positions.reserve(config.targetCount);

		const float minWorldX = static_cast<float>(m_config.chunkMinX) * m_config.chunkSize + 4.0f;
		const float maxWorldX = static_cast<float>(m_config.chunkMaxX + 1) * m_config.chunkSize - 4.0f;
		const float minWorldZ = static_cast<float>(m_config.chunkMinZ) * m_config.chunkSize + 4.0f;
		const float maxWorldZ = static_cast<float>(m_config.chunkMaxZ + 1) * m_config.chunkSize - 4.0f;
		const float minSpacingSq = config.minSpacing * config.minSpacing;

		SeededRng		   rng(config.seed);
		constexpr uint32_t MAX_ATTEMPTS = 4096u;

		for (uint32_t attempt = 0; attempt < MAX_ATTEMPTS && positions.size() < config.targetCount; ++attempt)
		{
			const float wx = rng.NextRange(minWorldX, maxWorldX);
			const float wz = rng.NextRange(minWorldZ, maxWorldZ);

			if (wx >= config.exclusionMinX && wx <= config.exclusionMaxX && wz >= config.exclusionMinZ && wz <= config.exclusionMaxZ)
				continue;

			const float wy = m_generator.SampleHeight(wx, wz);
			if (wy < m_config.waterLevel + 0.35f)
				continue;

			const float slope = m_generator.SampleSlope(wx, wz);
			if (slope > config.maxSlope)
				continue;

			bool tooClose = false;
			for (const Vector3& existing : positions)
			{
				const float dx = wx - existing.GetX();
				const float dz = wz - existing.GetZ();
				if (dx * dx + dz * dz < minSpacingSq)
				{
					tooClose = true;
					break;
				}
			}
			if (tooClose)
				continue;

			positions.emplace_back(wx, wy, wz);
		}

		return positions;
	}

	void TerrainSystem::BuildTerrainAndWater(Scene* scene)
	{
		if (!scene || !scene->GetResourceManager())
			return;

		ResourceManager* rm = scene->GetResourceManager();

		auto loadOrDefault = [&](const std::string& path, const ResourceId fallback) -> ResourceId {
			if (!path.empty() && std::filesystem::exists(AssetPath::Resolve(path)))
				return rm->LoadTexture(path);
			return fallback;
		};

		TerrainMaterialCreateInfo matInfo{};
		matInfo.name = "SponzaTerrainMaterial";
		for (size_t i = 0; i < 5u; ++i)
		{
			const std::string& albedoPath = !s_textureOverrides[i].empty() ? s_textureOverrides[i] : m_config.layers[i].albedoPath;
			matInfo.albedoTextures[i] = loadOrDefault(albedoPath, rm->GetDefaultTexture());
			matInfo.normalTextures[i] = loadOrDefault(m_config.layers[i].normalPath, rm->GetDefaultNormalTexture());
			matInfo.roughnessTextures[i] = loadOrDefault(m_config.layers[i].roughnessPath, rm->GetDefaultMetallicRoughnessTexture());
		}

		const ResourceId terrainMatId = rm->CreateTerrainMaterial(matInfo);

		for (int32_t cz = m_config.chunkMinZ; cz <= m_config.chunkMaxZ; ++cz)
		{
			for (int32_t cx = m_config.chunkMinX; cx <= m_config.chunkMaxX; ++cx)
			{
				const MeshData terrainMesh = m_generator.GenerateChunkMesh(cx, cz);
				if (!terrainMesh.vertices.empty() && !terrainMesh.indices.empty())
				{
					SceneObject		obj = scene->CreateObject("TerrainChunk_" + std::to_string(cx) + "_" + std::to_string(cz));
					MeshComponent&	mc = obj.AddComponent<MeshComponent>();
					mc.meshId = rm->CreateMesh(terrainMesh);
					mc.materialId = terrainMatId;
				}

				const MeshData grassMesh = m_generator.GenerateGrassChunkMesh(cx, cz);
				if (!grassMesh.vertices.empty() && !grassMesh.indices.empty())
				{
					SceneObject	   grassObj = scene->CreateObject("GrassChunk_" + std::to_string(cx) + "_" + std::to_string(cz));
					MeshComponent& mc = grassObj.AddComponent<MeshComponent>();
					mc.meshId = rm->CreateMesh(grassMesh);
					grassObj.AddComponent<GrassChunkComponent>();
				}

				const MeshData waterMesh = m_generator.GenerateWaterChunkMesh(cx, cz);
				if (!waterMesh.vertices.empty() && !waterMesh.indices.empty())
				{
					SceneObject	   waterObj = scene->CreateObject("WaterChunk_" + std::to_string(cx) + "_" + std::to_string(cz));
					MeshComponent& mc = waterObj.AddComponent<MeshComponent>();
					mc.meshId = rm->CreateMesh(waterMesh);
					waterObj.AddComponent<WaterChunkComponent>();
				}
			}
		}

		Logger::Log<INFO>("TerrainSystem: built terrain, grass, and water chunks");
	}

	void TerrainSystem::ScatterTrees(Scene* scene, const TreeScatterConfig& config)
	{
		if (!scene || config.modelPath.empty())
			return;

		if (!std::filesystem::exists(AssetPath::Resolve(config.modelPath)))
		{
			Logger::Log<WARNING>("TerrainSystem: tree model '{}' not found; skipping tree scatter", config.modelPath);
			return;
		}

		const std::vector<Vector3> positions = ComputeTreePositions(config);
		SeededRng				   rng(config.seed + 555u);

		for (const Vector3& pos : positions)
		{
			SceneObject tree = scene->LoadModel(config.modelPath, true);
			const float yaw = rng.NextRange(0.0f, 360.0f);
			const float scale = rng.NextRange(config.minScale, config.maxScale);
			tree.GetTransform().SetPosition(pos);
			tree.GetTransform().SetEulerDegrees(Vector3(0.0f, yaw, 0.0f));
			tree.GetTransform().SetScale(Vector3(scale, scale, scale));
		}

		Logger::Log<INFO>("TerrainSystem: scattered {} trees", positions.size());
	}
}
