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

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace BetterThanNothing
{
	struct TerrainFlattenZone
	{
		float minX = -19.4f;
		float maxX = 18.2f;
		float minZ = -12.0f;
		float maxZ = 11.2f;
		float targetHeight = -0.18f;
		float blendMargin = 8.0f;
	};

	struct TerrainWaterBasin
	{
		float centerX = 0.0f;
		float centerZ = 0.0f;
		float radiusX = 24.0f;
		float radiusZ = 20.0f;
		float depth = 2.6f;
		float blendMargin = 10.0f;
	};

	struct TerrainLayerTextures
	{
		std::string albedoPath;
		std::string normalPath;
		std::string roughnessPath;
	};

	struct TerrainConfig
	{
		uint32_t seed = 1337u;
		int32_t	 chunkMinX = -4;
		int32_t	 chunkMaxX = 3;
		int32_t	 chunkMinZ = -4;
		int32_t	 chunkMaxZ = 3;
		float	 chunkSize = 32.0f;
		uint32_t quadsPerChunk = 32u;
		float	 waterLevel = -1.60f;
		float	 hillAmplitude = 10.0f;
		float	 hillFrequency = 0.012f;

		TerrainFlattenZone					sponzaPlateau{};
		std::vector<TerrainWaterBasin>		waterBasins;
		std::array<TerrainLayerTextures, 5> layers{};

		[[nodiscard]] static TerrainConfig CreateSponzaLandscapeDefaults()
		{
			TerrainConfig cfg{};
			cfg.seed = 1337u;
			cfg.chunkMinX = -4;
			cfg.chunkMaxX = 3;
			cfg.chunkMinZ = -4;
			cfg.chunkMaxZ = 3;
			cfg.chunkSize = 32.0f;
			cfg.quadsPerChunk = 32u;
			cfg.waterLevel = -1.60f;
			cfg.hillAmplitude = 10.0f;
			cfg.hillFrequency = 0.012f;

			cfg.sponzaPlateau = {
				.minX = -19.4f,
				.maxX = 18.2f,
				.minZ = -12.0f,
				.maxZ = 11.2f,
				.targetHeight = -0.18f,
				.blendMargin = 8.0f,
			};

			cfg.waterBasins = {
				{.centerX = 48.0f, .centerZ = 0.0f, .radiusX = 22.0f, .radiusZ = 18.0f, .depth = 2.6f, .blendMargin = 10.0f},
				{.centerX = -50.0f, .centerZ = 4.0f, .radiusX = 24.0f, .radiusZ = 20.0f, .depth = 2.8f, .blendMargin = 10.0f},
				{.centerX = 2.0f, .centerZ = 46.0f, .radiusX = 26.0f, .radiusZ = 18.0f, .depth = 2.5f, .blendMargin = 10.0f},
				{.centerX = -4.0f, .centerZ = -46.0f, .radiusX = 24.0f, .radiusZ = 19.0f, .depth = 2.7f, .blendMargin = 10.0f},
			};

			// 0: Dirt (Ground049A), 1: Grass (Grass002), 2: Rock (Ground071), 3: Mud (Ground086), 4: Sand (Ground108)
			cfg.layers[0] = {"Assets/Textures/Terrain/Ground049A/Ground049A_1K-JPG_Color.jpg", "Assets/Textures/Terrain/Ground049A/Ground049A_1K-JPG_NormalGL.jpg", "Assets/Textures/Terrain/Ground049A/Ground049A_1K-JPG_Roughness.jpg"};
			cfg.layers[1] = {"Assets/Textures/Terrain/Grass002/Grass002_1K-JPG_Color.jpg", "Assets/Textures/Terrain/Grass002/Grass002_1K-JPG_NormalGL.jpg", "Assets/Textures/Terrain/Grass002/Grass002_1K-JPG_Roughness.jpg"};
			cfg.layers[2] = {"Assets/Textures/Terrain/Ground071/Ground071_1K-JPG_Color.jpg", "Assets/Textures/Terrain/Ground071/Ground071_1K-JPG_NormalGL.jpg", "Assets/Textures/Terrain/Ground071/Ground071_1K-JPG_Roughness.jpg"};
			cfg.layers[3] = {"Assets/Textures/Terrain/Ground086/Ground086_1K-JPG_Color.jpg", "Assets/Textures/Terrain/Ground086/Ground086_1K-JPG_NormalGL.jpg", "Assets/Textures/Terrain/Ground086/Ground086_1K-JPG_Roughness.jpg"};
			cfg.layers[4] = {"Assets/Textures/Terrain/Ground108/Ground108_1K-JPG_Color.jpg", "Assets/Textures/Terrain/Ground108/Ground108_1K-JPG_NormalGL.jpg", "Assets/Textures/Terrain/Ground108/Ground108_1K-JPG_Roughness.jpg"};

			return cfg;
		}
	};

	struct TreeScatterConfig
	{
		uint32_t	targetCount = 95u;
		float		minSpacing = 8.5f;
		float		exclusionMinX = -22.0f;
		float		exclusionMaxX = 21.0f;
		float		exclusionMinZ = -15.0f;
		float		exclusionMaxZ = 14.0f;
		float		maxSlope = 0.25f;
		float		minScale = 0.85f;
		float		maxScale = 1.35f;
		uint32_t	seed = 4242u;
		std::string modelPath = "Assets/Models/tree_elm/scene.gltf";
	};
}
