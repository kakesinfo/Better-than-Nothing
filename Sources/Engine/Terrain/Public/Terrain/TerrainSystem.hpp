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

#include <Math/Vector3.hpp>
#include <Terrain/TerrainConfig.hpp>
#include <Terrain/TerrainGenerator.hpp>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace BetterThanNothing
{
	class Scene;

	class TerrainSystem
	{
	public:
		enum class TerrainTextureType : uint32_t
		{
			DirtTexture = 0,
			GrassTexture = 1,
			RockTexture = 2,
			MudTexture = 3,
			SandTexture = 4
		};

		static constexpr TerrainTextureType DirtTexture = TerrainTextureType::DirtTexture;
		static constexpr TerrainTextureType GrassTexture = TerrainTextureType::GrassTexture;
		static constexpr TerrainTextureType RockTexture = TerrainTextureType::RockTexture;
		static constexpr TerrainTextureType MudTexture = TerrainTextureType::MudTexture;
		static constexpr TerrainTextureType SandTexture = TerrainTextureType::SandTexture;

		explicit TerrainSystem(const TerrainConfig& config = TerrainConfig::CreateSponzaLandscapeDefaults());

		static void								 mapTexture(TerrainTextureType type, const std::string& texturePath);
		[[nodiscard]] static const std::array<std::string, 5>& GetTextureOverrides();

		[[nodiscard]] std::vector<Vector3> ComputeTreePositions(const TreeScatterConfig& config = TreeScatterConfig{}) const;

		void BuildTerrainAndWater(Scene* scene);
		void ScatterTrees(Scene* scene, const TreeScatterConfig& config = TreeScatterConfig{});

		[[nodiscard]] const TerrainGenerator& GetGenerator() const noexcept { return m_generator; }

	private:
		TerrainConfig	 m_config;
		TerrainGenerator m_generator;

		static std::array<std::string, 5> s_textureOverrides;
	};
}
