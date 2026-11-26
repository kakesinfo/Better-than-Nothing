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

	MeshData TerrainGenerator::GenerateGrassChunkMesh(const int32_t chunkX, const int32_t chunkZ) const
	{
		MeshData				  mesh{};
		constexpr uint32_t		  TUFTS_PER_CHUNK = 2600u;
		constexpr uint32_t		  BLADES_PER_TUFT = 4u;
		const float				  originX = static_cast<float>(chunkX) * m_config.chunkSize;
		const float				  originZ = static_cast<float>(chunkZ) * m_config.chunkSize;
		const TerrainFlattenZone& sponza = m_config.sponzaPlateau;

		const uint32_t chunkSeed = Hash2D(chunkX, chunkZ, m_config.seed + 9001u);
		SeededRng	   rng(chunkSeed);

		mesh.vertices.reserve(TUFTS_PER_CHUNK * BLADES_PER_TUFT * 5u);
		mesh.indices.reserve(TUFTS_PER_CHUNK * BLADES_PER_TUFT * 9u);

		for (uint32_t t = 0; t < TUFTS_PER_CHUNK; ++t)
		{
			const float tuftX = originX + rng.NextRange(0.0f, m_config.chunkSize);
			const float tuftZ = originZ + rng.NextRange(0.0f, m_config.chunkSize);

			// Exclude Sponza interior
			if (tuftX >= sponza.minX && tuftX <= sponza.maxX && tuftZ >= sponza.minZ && tuftZ <= sponza.maxZ)
				continue;

			const float terrainY = SampleHeight(tuftX, tuftZ);
			if (terrainY < m_config.waterLevel + 0.22f)
				continue;

			const Vector3 normal = SampleNormal(tuftX, tuftZ);
			const float	  slope = std::clamp(1.0f - normal.GetY(), 0.0f, 1.0f);
			if (slope > 0.28f)
				continue;

			for (uint32_t b = 0; b < BLADES_PER_TUFT; ++b)
			{
				const float bx = tuftX + rng.NextRange(-0.18f, 0.18f);
				const float bz = tuftZ + rng.NextRange(-0.18f, 0.18f);
				if (bx >= sponza.minX - 0.05f && bx <= sponza.maxX + 0.05f && bz >= sponza.minZ - 0.05f && bz <= sponza.maxZ + 0.05f)
					continue;
				const float by = SampleHeight(bx, bz);
				if (by < m_config.waterLevel + 0.22f)
					continue;
				const float angle = rng.NextRange(0.0f, 6.2831853f);
				const float height = rng.NextRange(0.32f, 0.68f);
				const float width = rng.NextRange(0.025f, 0.045f);
				const float bend = rng.NextRange(0.06f, 0.18f);
				const float phase = rng.NextFloat();
				const float tint = rng.NextRange(0.75f, 1.15f);

				const float cosA = std::cos(angle);
				const float sinA = std::sin(angle);
				const float rx = cosA * width;
				const float rz = sinA * width;
				const float fx = -sinA * bend;
				const float fz = cosA * bend;

				const uint32_t baseIdx = static_cast<uint32_t>(mesh.vertices.size());

				auto pushVert = [&](const float px, const float py, const float pz, const float u, const float v) {
					VertexData vert{};
					vert.position = Vector3(px, py, pz);
					vert.normal = normal;
					vert.uv = Vector2(u, v);
					vert.color = Vector3(phase, tint, height);
					mesh.vertices.push_back(vert);
				};

				// 5 vertices: base L/R, mid L/R, tip
				pushVert(bx - rx, by, bz - rz, 0.0f, 0.0f);
				pushVert(bx + rx, by, bz + rz, 1.0f, 0.0f);
				pushVert(bx - rx * 0.55f + fx * 0.35f, by + height * 0.52f, bz - rz * 0.55f + fz * 0.35f, 0.0f, 0.5f);
				pushVert(bx + rx * 0.55f + fx * 0.35f, by + height * 0.52f, bz + rz * 0.55f + fz * 0.35f, 1.0f, 0.5f);
				pushVert(bx + fx, by + height, bz + fz, 0.5f, 1.0f);

				// 9 indices (3 triangles)
				mesh.indices.push_back(baseIdx + 0u);
				mesh.indices.push_back(baseIdx + 2u);
				mesh.indices.push_back(baseIdx + 1u);

				mesh.indices.push_back(baseIdx + 1u);
				mesh.indices.push_back(baseIdx + 2u);
				mesh.indices.push_back(baseIdx + 3u);

				mesh.indices.push_back(baseIdx + 2u);
				mesh.indices.push_back(baseIdx + 4u);
				mesh.indices.push_back(baseIdx + 3u);
			}
		}

		return mesh;
	}
}
