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

#pragma once

#include "TerrainConfig.hpp"
#include <Data/Graphics/MeshData.hpp>
#include <Math/Vector3.hpp>
#include <cstdint>

namespace BetterThanNothing
{
	class TerrainGenerator
	{
	public:
		explicit TerrainGenerator(TerrainConfig config);

		[[nodiscard]] const TerrainConfig& GetConfig() const { return m_config; }

		[[nodiscard]] float	  SampleHeight(float worldX, float worldZ) const;
		[[nodiscard]] Vector3 SampleNormal(float worldX, float worldZ) const;
		[[nodiscard]] float	  SampleSlope(float worldX, float worldZ) const;

		// Returns (wSand, wGrass, wDirt); rocky weight is implicitly 1 - (r + g + b)
		[[nodiscard]] Vector3 ComputeSplatWeights(float worldX, float worldZ, float height, float slope) const;

		// Generates local-space mesh for chunk (chunkX, chunkZ) centered at chunk origin
		[[nodiscard]] MeshData GenerateChunkMesh(int32_t chunkX, int32_t chunkZ) const;

		// Generates water mesh for chunk (chunkX, chunkZ) only if terrain dips below waterLevel; empty otherwise
		[[nodiscard]] MeshData GenerateWaterChunkMesh(int32_t chunkX, int32_t chunkZ) const;

		// Generates procedural 3D grass blades for chunk (chunkX, chunkZ) on grassy meadows outside Sponza
		[[nodiscard]] MeshData GenerateGrassChunkMesh(int32_t chunkX, int32_t chunkZ) const;

		[[nodiscard]] float GetWorldOriginX() const;
		[[nodiscard]] float GetWorldOriginZ() const;
		[[nodiscard]] float GetWorldWidth() const;
		[[nodiscard]] float GetWorldDepth() const;

	private:
		TerrainConfig m_config;
	};
}
