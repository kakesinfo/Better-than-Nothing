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

#pragma once

#include <Data/Graphics/MeshData.hpp>
#include <Math/Vector3.hpp>
#include <Terrain/TerrainConfig.hpp>
#include <array>
#include <cstdint>

namespace BetterThanNothing
{
	class TerrainGenerator
	{
	public:
		explicit TerrainGenerator(const TerrainConfig& config = TerrainConfig::CreateSponzaLandscapeDefaults());

		[[nodiscard]] const TerrainConfig& GetConfig() const noexcept { return m_config; }

		[[nodiscard]] float				   SampleHeight(float worldX, float worldZ) const;
		[[nodiscard]] Vector3			   SampleNormal(float worldX, float worldZ) const;
		[[nodiscard]] float				   SampleSlope(float worldX, float worldZ) const;
		[[nodiscard]] std::array<float, 5> ComputeSplatWeights(float worldX, float worldZ, float height, float slope) const;

		[[nodiscard]] MeshData GenerateChunkMesh(int32_t chunkX, int32_t chunkZ) const;
		[[nodiscard]] MeshData GenerateWaterChunkMesh(int32_t chunkX, int32_t chunkZ) const;

	private:
		TerrainConfig m_config;
	};
}
