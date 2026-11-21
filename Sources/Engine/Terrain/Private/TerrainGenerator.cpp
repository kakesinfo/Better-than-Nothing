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

#include "Terrain/TerrainGenerator.hpp"

#include "Terrain/Noise.hpp"
#include <algorithm>
#include <cmath>

namespace BetterThanNothing
{
	TerrainGenerator::TerrainGenerator(TerrainConfig config) : m_config(std::move(config)) {}

	float TerrainGenerator::GetWorldWidth() const
	{
		return static_cast<float>(m_config.chunksX) * m_config.chunkSize;
	}

	float TerrainGenerator::GetWorldDepth() const
	{
		return static_cast<float>(m_config.chunksZ) * m_config.chunkSize;
	}

	float TerrainGenerator::GetWorldOriginX() const
	{
		return -GetWorldWidth() * 0.5f;
	}

	float TerrainGenerator::GetWorldOriginZ() const
	{
		return -GetWorldDepth() * 0.5f;
	}

	float TerrainGenerator::SampleHeight(const float worldX, const float worldZ) const
	{
		const float freq = m_config.baseFrequency;

		// Subtle domain warp for natural organic ridges and valleys
		const float warpX = TerrainNoise::FBM2D(worldX * freq * 0.65f + 13.7f, worldZ * freq * 0.65f - 7.3f, m_config.seed + 17u, 3) * 14.0f;
		const float warpZ = TerrainNoise::FBM2D(worldX * freq * 0.65f - 21.4f, worldZ * freq * 0.65f + 19.1f, m_config.seed + 53u, 3) * 14.0f;

		const float wx = worldX + warpX;
		const float wz = worldZ + warpZ;

		// Rolling hills + fine ground detail
		const float hills = TerrainNoise::FBM2D(wx * freq, wz * freq, m_config.seed, 5, 2.04f, 0.48f);
		const float detail = TerrainNoise::FBM2D(worldX * freq * 3.8f, worldZ * freq * 3.8f, m_config.seed + 311u, 3, 2.1f, 0.45f);

		// Perimeter mountain amphitheater rising gradually beyond ~60m from Sponza
		const float radialDist = std::sqrt(worldX * worldX + worldZ * worldZ);
		const float ringMask = TerrainNoise::QuinticSmoothstep(55.0f, 128.0f, radialDist);
		const float ridges = TerrainNoise::RidgedFBM2D(wx * freq * 0.72f, wz * freq * 0.72f, m_config.seed + 733u, 4);

		float height = hills * m_config.hillAmplitude + detail * 1.15f + ringMask * ridges * m_config.mountainAmplitude;

		// Carve organic water basins (lakes & ponds)
		for (size_t i = 0; i < m_config.waterBasins.size(); ++i)
		{
			const auto& basin = m_config.waterBasins[i];
			const float dx = (worldX - basin.centerX) / std::max(basin.radiusX, 1.0f);
			const float dz = (worldZ - basin.centerZ) / std::max(basin.radiusZ, 1.0f);

			const float shoreWarp = TerrainNoise::FBM2D(
				worldX * 0.045f + static_cast<float>(i) * 17.0f,
				worldZ * 0.045f - static_cast<float>(i) * 11.0f,
				m_config.seed + 900u + static_cast<uint32_t>(i) * 37u,
				3
			) * 0.28f;

			const float normDist = std::sqrt(dx * dx + dz * dz) + shoreWarp;
			const float carve = 1.0f - TerrainNoise::QuinticSmoothstep(0.18f, 1.05f, normDist);
			if (carve > 0.0f)
			{
				const float lakeBedHeight = m_config.waterLevel - basin.depth * (0.45f + 0.55f * carve);
				height = height * (1.0f - carve) + lakeBedHeight * carve;
			}
		}

		// Apply smooth rectangular plateau flatten zones (e.g. Sponza footprint)
		for (const auto& zone : m_config.flattenZones)
		{
			const float ex = std::max(std::abs(worldX - zone.centerX) - zone.halfExtentX, 0.0f);
			const float ez = std::max(std::abs(worldZ - zone.centerZ) - zone.halfExtentZ, 0.0f);
			const float distOutside = std::sqrt(ex * ex + ez * ez);
			const float flattenFactor = 1.0f - TerrainNoise::QuinticSmoothstep(0.0f, std::max(zone.blendRadius, 0.1f), distOutside);
			height = height * (1.0f - flattenFactor) + zone.targetHeight * flattenFactor;
		}

		return height;
	}

	Vector3 TerrainGenerator::SampleNormal(const float worldX, const float worldZ) const
	{
		constexpr float eps = 0.35f;
		const float		hL = SampleHeight(worldX - eps, worldZ);
		const float		hR = SampleHeight(worldX + eps, worldZ);
		const float		hD = SampleHeight(worldX, worldZ - eps);
		const float		hU = SampleHeight(worldX, worldZ + eps);
		return Vector3(hL - hR, 2.0f * eps, hD - hU).Normalized();
	}

	float TerrainGenerator::SampleSlope(const float worldX, const float worldZ) const
	{
		const Vector3 n = SampleNormal(worldX, worldZ);
		return std::clamp(1.0f - n.GetY(), 0.0f, 1.0f);
	}

	Vector3 TerrainGenerator::ComputeSplatWeights(const float worldX, const float worldZ, const float height, const float slope) const
	{
		const float n1 = TerrainNoise::FBM2D(worldX * 0.055f, worldZ * 0.055f, m_config.seed + 4129u, 3) * 0.5f + 0.5f;
		const float n2 = TerrainNoise::FBM2D(worldX * 0.03f + 41.0f, worldZ * 0.03f - 19.0f, m_config.seed + 6151u, 3) * 0.5f + 0.5f;

		// Layer 0 (R): Sand / Riverbed & Shoreline
		const float heightAboveWater = height - m_config.waterLevel;
		float		wSand = 1.0f - TerrainNoise::Smoothstep(-0.35f, 1.35f + (n1 - 0.5f) * 0.7f, heightAboveWater);

		// Layer 3 (1 - R - G - B): Rocky Cliffs / Gravel on steep slopes & high peaks
		const float slopeRock = TerrainNoise::Smoothstep(0.14f, 0.36f, slope + (n1 - 0.5f) * 0.06f);
		const float highRock = TerrainNoise::Smoothstep(10.0f, 20.0f, height + (n2 - 0.5f) * 4.0f) * 0.85f;
		float		wRock = std::clamp(std::max(slopeRock, highRock), 0.0f, 1.0f);

		// Layer 2 (B): Forest Dirt around Sponza edges and organic forest soil patches
		float courtyardDirt = 0.0f;
		for (const auto& zone : m_config.flattenZones)
		{
			const float ex = std::max(std::abs(worldX - zone.centerX) - (zone.halfExtentX + 2.0f), 0.0f);
			const float ez = std::max(std::abs(worldZ - zone.centerZ) - (zone.halfExtentZ + 2.0f), 0.0f);
			const float d = std::sqrt(ex * ex + ez * ez);
			courtyardDirt = std::max(courtyardDirt, (1.0f - TerrainNoise::Smoothstep(0.0f, 14.0f, d)) * 0.85f);
		}
		const float soilPatch = TerrainNoise::Smoothstep(0.54f, 0.82f, n2) * 0.45f;
		float		wDirt = std::clamp(std::max(courtyardDirt * 0.65f, soilPatch), 0.0f, 1.0f);

		// Layer 1 (G): Lush Grass002 fills remaining flat/gentle lowlands & meadows
		float wGrass = std::max(0.0f, 1.0f - wDirt);

		// Priority: rock dominates steep slopes, sand dominates shorelines/underwater
		const float nonRock = std::max(0.0f, 1.0f - wRock);
		wSand *= nonRock;
		const float remaining = std::max(0.0f, nonRock - wSand);
		const float gdSum = std::max(wGrass + wDirt, 1e-5f);
		wGrass = (wGrass / gdSum) * remaining;
		wDirt = (wDirt / gdSum) * remaining;

		// Ensure R + G + B <= 1.0 so shader computes wRock = max(0, 1 - R - G - B) cleanly
		const float rgbSum = wSand + wGrass + wDirt;
		if (rgbSum > 1.0f)
		{
			const float inv = 1.0f / rgbSum;
			wSand *= inv;
			wGrass *= inv;
			wDirt *= inv;
		}

		return Vector3(wSand, wGrass, wDirt);
	}

	MeshData TerrainGenerator::GenerateChunkMesh(const int32_t chunkX, const int32_t chunkZ) const
	{
		const uint32_t res = std::max(2u, m_config.chunkResolution);
		const uint32_t vertsPerAxis = res + 1u;
		const float	   step = m_config.chunkSize / static_cast<float>(res);

		const float chunkOriginX = GetWorldOriginX() + static_cast<float>(chunkX) * m_config.chunkSize;
		const float chunkOriginZ = GetWorldOriginZ() + static_cast<float>(chunkZ) * m_config.chunkSize;

		MeshData mesh{};
		mesh.vertices.reserve(static_cast<size_t>(vertsPerAxis) * vertsPerAxis);
		mesh.indices.reserve(static_cast<size_t>(res) * res * 6u);

		for (uint32_t iz = 0; iz <= res; ++iz)
		{
			const float localZ = static_cast<float>(iz) * step;
			const float worldZ = chunkOriginZ + localZ;

			for (uint32_t ix = 0; ix <= res; ++ix)
			{
				const float localX = static_cast<float>(ix) * step;
				const float worldX = chunkOriginX + localX;

				const float	  h = SampleHeight(worldX, worldZ);
				const Vector3 n = SampleNormal(worldX, worldZ);
				const float	  slope = std::clamp(1.0f - n.GetY(), 0.0f, 1.0f);
				const Vector3 splat = ComputeSplatWeights(worldX, worldZ, h, slope);

				VertexData v{};
				v.position = Vector3(localX, h, localZ);
				v.color = splat;
				v.uv = Vector2(worldX * m_config.uvScale, worldZ * m_config.uvScale);
				v.normal = n;
				mesh.vertices.push_back(v);
			}
		}

		// Counter-clockwise winding viewed from +Y (matches Vulkan front-face with Y-flipped projection)
		for (uint32_t iz = 0; iz < res; ++iz)
		{
			for (uint32_t ix = 0; ix < res; ++ix)
			{
				const uint32_t i00 = iz * vertsPerAxis + ix;
				const uint32_t i10 = i00 + 1u;
				const uint32_t i01 = (iz + 1u) * vertsPerAxis + ix;
				const uint32_t i11 = i01 + 1u;

				mesh.indices.push_back(i00);
				mesh.indices.push_back(i01);
				mesh.indices.push_back(i10);

				mesh.indices.push_back(i10);
				mesh.indices.push_back(i01);
				mesh.indices.push_back(i11);
			}
		}

		return mesh;
	}

	MeshData TerrainGenerator::GenerateWaterChunkMesh(const int32_t chunkX, const int32_t chunkZ) const
	{
		const uint32_t res = std::max(2u, m_config.waterResolution);
		const uint32_t vertsPerAxis = res + 1u;
		const float	   step = m_config.chunkSize / static_cast<float>(res);

		const float chunkOriginX = GetWorldOriginX() + static_cast<float>(chunkX) * m_config.chunkSize;
		const float chunkOriginZ = GetWorldOriginZ() + static_cast<float>(chunkZ) * m_config.chunkSize;

		std::vector<float> depths(static_cast<size_t>(vertsPerAxis) * vertsPerAxis, 0.0f);
		bool			   hasWater = false;

		for (uint32_t iz = 0; iz <= res; ++iz)
		{
			const float worldZ = chunkOriginZ + static_cast<float>(iz) * step;
			for (uint32_t ix = 0; ix <= res; ++ix)
			{
				const float worldX = chunkOriginX + static_cast<float>(ix) * step;
				const float terrainH = SampleHeight(worldX, worldZ);
				const float depth = m_config.waterLevel - terrainH;
				depths[static_cast<size_t>(iz) * vertsPerAxis + ix] = depth;
				if (depth > -0.25f)
					hasWater = true;
			}
		}

		MeshData mesh{};
		if (!hasWater)
			return mesh;

		mesh.vertices.reserve(static_cast<size_t>(vertsPerAxis) * vertsPerAxis);
		mesh.indices.reserve(static_cast<size_t>(res) * res * 6u);

		for (uint32_t iz = 0; iz <= res; ++iz)
		{
			const float localZ = static_cast<float>(iz) * step;
			const float worldZ = chunkOriginZ + localZ;
			for (uint32_t ix = 0; ix <= res; ++ix)
			{
				const float localX = static_cast<float>(ix) * step;
				const float worldX = chunkOriginX + localX;
				const float depth = std::max(depths[static_cast<size_t>(iz) * vertsPerAxis + ix], 0.0f);

				VertexData v{};
				v.position = Vector3(localX, m_config.waterLevel, localZ);
				// Encode local water depth in color.r so Gerstner vertex shader can damp waves near shore
				v.color = Vector3(depth, 0.0f, 0.0f);
				v.uv = Vector2(worldX * 0.1f, worldZ * 0.1f);
				v.normal = Vector3(0.0f, 1.0f, 0.0f);
				mesh.vertices.push_back(v);
			}
		}

		for (uint32_t iz = 0; iz < res; ++iz)
		{
			for (uint32_t ix = 0; ix < res; ++ix)
			{
				const uint32_t i00 = iz * vertsPerAxis + ix;
				const uint32_t i10 = i00 + 1u;
				const uint32_t i01 = (iz + 1u) * vertsPerAxis + ix;
				const uint32_t i11 = i01 + 1u;

				// Only emit quads where at least one corner is near or below water level
				if (depths[i00] <= -0.5f && depths[i10] <= -0.5f && depths[i01] <= -0.5f && depths[i11] <= -0.5f)
					continue;

				mesh.indices.push_back(i00);
				mesh.indices.push_back(i01);
				mesh.indices.push_back(i10);

				mesh.indices.push_back(i10);
				mesh.indices.push_back(i01);
				mesh.indices.push_back(i11);
			}
		}

		if (mesh.indices.empty())
			mesh.vertices.clear();

		return mesh;
	}
}
