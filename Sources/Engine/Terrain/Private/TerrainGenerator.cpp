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

#include <Terrain/Noise.hpp>
#include <Terrain/TerrainGenerator.hpp>
#include <algorithm>
#include <cmath>

namespace BetterThanNothing
{
	TerrainGenerator::TerrainGenerator(const TerrainConfig& config)
		: m_config(config)
	{
	}

	float TerrainGenerator::SampleHeight(const float worldX, const float worldZ) const
	{
		const float fbm = FBM2D(worldX * m_config.hillFrequency, worldZ * m_config.hillFrequency, 4, 2.0f, 0.5f, m_config.seed);
		const float ridge = RidgedFBM2D(worldX * m_config.hillFrequency * 0.65f, worldZ * m_config.hillFrequency * 0.65f, 3, 2.0f, 0.5f, m_config.seed + 77u);
		float		height = fbm * m_config.hillAmplitude + (ridge - 0.35f) * 3.0f;

		for (const TerrainWaterBasin& basin : m_config.waterBasins)
		{
			const float dx = (worldX - basin.centerX) / std::max(basin.radiusX, 1e-3f);
			const float dz = (worldZ - basin.centerZ) / std::max(basin.radiusZ, 1e-3f);
			const float normDist = std::sqrt(dx * dx + dz * dz);
			const float outerRadius = 1.0f + basin.blendMargin / std::max(std::min(basin.radiusX, basin.radiusZ), 1.0f);
			if (normDist < outerRadius)
			{
				const float t = QuinticSmoothstep(1.0f - std::clamp((normDist - 1.0f) / std::max(outerRadius - 1.0f, 1e-3f), 0.0f, 1.0f));
				const float basinBottom = m_config.waterLevel - basin.depth;
				height = height + t * (std::min(height, basinBottom) - height);
			}
		}

		const TerrainFlattenZone& flat = m_config.sponzaPlateau;
		const float				  dx = std::max({flat.minX - worldX, 0.0f, worldX - flat.maxX});
		const float				  dz = std::max({flat.minZ - worldZ, 0.0f, worldZ - flat.maxZ});
		const float				  distToBox = std::sqrt(dx * dx + dz * dz);
		if (distToBox < flat.blendMargin)
		{
			const float t = QuinticSmoothstep(1.0f - std::clamp(distToBox / std::max(flat.blendMargin, 1e-3f), 0.0f, 1.0f));
			height = height + t * (flat.targetHeight - height);
		}

		return height;
	}

	Vector3 TerrainGenerator::SampleNormal(const float worldX, const float worldZ) const
	{
		constexpr float eps = 0.25f;
		const float		hL = SampleHeight(worldX - eps, worldZ);
		const float		hR = SampleHeight(worldX + eps, worldZ);
		const float		hD = SampleHeight(worldX, worldZ - eps);
		const float		hU = SampleHeight(worldX, worldZ + eps);
		return Vector3(hL - hR, 2.0f * eps, hD - hU).Normalized();
	}

	float TerrainGenerator::SampleSlope(const float worldX, const float worldZ) const
	{
		return std::clamp(1.0f - SampleNormal(worldX, worldZ).GetY(), 0.0f, 1.0f);
	}

	std::array<float, 5> TerrainGenerator::ComputeSplatWeights(const float worldX, const float worldZ, const float height, const float slope) const
	{
		const float noiseVal = FBM2D(worldX * 0.05f, worldZ * 0.05f, 3, 2.0f, 0.5f, m_config.seed + 313u) * 0.5f + 0.5f;

		const float rock = QuinticSmoothstep(std::clamp((slope - 0.18f) / 0.22f, 0.0f, 1.0f));
		const float sand = QuinticSmoothstep(1.0f - std::clamp((height - (m_config.waterLevel + 0.35f)) / 0.65f, 0.0f, 1.0f));
		const float mud = QuinticSmoothstep(1.0f - std::clamp(std::abs(height - (m_config.waterLevel + 0.55f)) / 0.85f, 0.0f, 1.0f)) * (0.4f + 0.6f * noiseVal) * (1.0f - rock);
		const float dirt = QuinticSmoothstep(std::clamp((noiseVal - 0.55f) / 0.35f, 0.0f, 1.0f)) * (1.0f - rock) * 0.65f;
		const float grass = std::max(0.0f, 1.0f - std::min(1.0f, rock + sand + mud + dirt));

		// [0: Dirt, 1: Grass, 2: Rock, 3: Mud, 4: Sand]
		std::array<float, 5> weights = {dirt, grass, rock, mud, sand};
		const float			 sum = weights[0] + weights[1] + weights[2] + weights[3] + weights[4];
		if (sum > 1e-5f)
		{
			for (float& w : weights)
				w /= sum;
		}
		else
		{
			weights = {0.0f, 1.0f, 0.0f, 0.0f, 0.0f};
		}
		return weights;
	}

	MeshData TerrainGenerator::GenerateChunkMesh(const int32_t chunkX, const int32_t chunkZ) const
	{
		MeshData	   mesh{};
		const uint32_t quads = m_config.quadsPerChunk;
		const uint32_t vertsPerSide = quads + 1u;
		const float	   step = m_config.chunkSize / static_cast<float>(quads);
		const float	   originX = static_cast<float>(chunkX) * m_config.chunkSize;
		const float	   originZ = static_cast<float>(chunkZ) * m_config.chunkSize;

		mesh.vertices.reserve(static_cast<size_t>(vertsPerSide) * vertsPerSide);
		mesh.indices.reserve(static_cast<size_t>(quads) * quads * 6u);

		for (uint32_t z = 0; z <= quads; ++z)
		{
			for (uint32_t x = 0; x <= quads; ++x)
			{
				const float				   wx = originX + static_cast<float>(x) * step;
				const float				   wz = originZ + static_cast<float>(z) * step;
				const float				   wy = SampleHeight(wx, wz);
				const Vector3			   normal = SampleNormal(wx, wz);
				const float				   slope = std::clamp(1.0f - normal.GetY(), 0.0f, 1.0f);
				const std::array<float, 5> splat = ComputeSplatWeights(wx, wz, wy, slope);

				VertexData v{};
				v.position = Vector3(wx, wy, wz);
				v.normal = normal;
				v.uv = Vector2(wx * 0.25f, wz * 0.25f);
				v.color = Vector3(splat[1], splat[2], splat[3]);
				mesh.vertices.push_back(v);
			}
		}

		for (uint32_t z = 0; z < quads; ++z)
		{
			for (uint32_t x = 0; x < quads; ++x)
			{
				const uint32_t i0 = z * vertsPerSide + x;
				const uint32_t i1 = i0 + 1u;
				const uint32_t i2 = i0 + vertsPerSide;
				const uint32_t i3 = i2 + 1u;

				mesh.indices.push_back(i0);
				mesh.indices.push_back(i2);
				mesh.indices.push_back(i1);

				mesh.indices.push_back(i1);
				mesh.indices.push_back(i2);
				mesh.indices.push_back(i3);
			}
		}

		return mesh;
	}

	MeshData TerrainGenerator::GenerateWaterChunkMesh(const int32_t chunkX, const int32_t chunkZ) const
	{
		MeshData	   mesh{};
		const uint32_t quads = m_config.quadsPerChunk;
		const float	   step = m_config.chunkSize / static_cast<float>(quads);
		const float	   originX = static_cast<float>(chunkX) * m_config.chunkSize;
		const float	   originZ = static_cast<float>(chunkZ) * m_config.chunkSize;
		const float	   waterY = m_config.waterLevel;

		for (uint32_t z = 0; z < quads; ++z)
		{
			for (uint32_t x = 0; x < quads; ++x)
			{
				const float x0 = originX + static_cast<float>(x) * step;
				const float x1 = x0 + step;
				const float z0 = originZ + static_cast<float>(z) * step;
				const float z1 = z0 + step;

				const float h00 = SampleHeight(x0, z0);
				const float h10 = SampleHeight(x1, z0);
				const float h01 = SampleHeight(x0, z1);
				const float h11 = SampleHeight(x1, z1);

				const float minH = std::min({h00, h10, h01, h11});
				if (minH > waterY + 0.35f)
					continue;

				const uint32_t baseIdx = static_cast<uint32_t>(mesh.vertices.size());

				auto pushWaterVert = [&](const float wx, const float wz, const float terrainH) {
					VertexData v{};
					v.position = Vector3(wx, waterY, wz);
					v.normal = Vector3(0.0f, 1.0f, 0.0f);
					v.uv = Vector2(wx * 0.1f, wz * 0.1f);
					v.color = Vector3(std::max(0.0f, waterY - terrainH), 0.0f, 0.0f);
					mesh.vertices.push_back(v);
				};

				pushWaterVert(x0, z0, h00);
				pushWaterVert(x1, z0, h10);
				pushWaterVert(x0, z1, h01);
				pushWaterVert(x1, z1, h11);

				mesh.indices.push_back(baseIdx + 0u);
				mesh.indices.push_back(baseIdx + 2u);
				mesh.indices.push_back(baseIdx + 1u);

				mesh.indices.push_back(baseIdx + 1u);
				mesh.indices.push_back(baseIdx + 2u);
				mesh.indices.push_back(baseIdx + 3u);
			}
		}

		return mesh;
	}
}
