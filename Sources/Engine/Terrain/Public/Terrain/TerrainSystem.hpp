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
#include "TerrainGenerator.hpp"
#include <Scene/SceneObject.hpp>
#include <cstdint>
#include <memory>

namespace BetterThanNothing
{
	class Scene;

	class TerrainSystem
	{
	public:
		explicit TerrainSystem(TerrainConfig config = TerrainConfig::CreateSponzaLandscapeDefaults());

		SceneObject BuildTerrainAndWater(Scene* scene);
		SceneObject ScatterTrees(Scene* scene, const TreeScatterConfig& scatterConfig = TreeScatterConfig{});

		[[nodiscard]] float GetHeightAt(float worldX, float worldZ) const { return m_generator.SampleHeight(worldX, worldZ); }
		[[nodiscard]] float GetSlopeAt(float worldX, float worldZ) const { return m_generator.SampleSlope(worldX, worldZ); }
		[[nodiscard]] float GetWaterLevel() const { return m_generator.GetConfig().waterLevel; }

		[[nodiscard]] uint32_t GetTerrainChunkCount() const { return m_terrainChunkCount; }
		[[nodiscard]] uint32_t GetWaterChunkCount() const { return m_waterChunkCount; }
		[[nodiscard]] uint32_t GetTreeCount() const { return m_treeCount; }

		[[nodiscard]] const TerrainGenerator& GetGenerator() const { return m_generator; }

	private:
		TerrainGenerator m_generator;
		uint32_t		 m_terrainChunkCount = 0;
		uint32_t		 m_waterChunkCount = 0;
		uint32_t		 m_treeCount = 0;
	};
}
