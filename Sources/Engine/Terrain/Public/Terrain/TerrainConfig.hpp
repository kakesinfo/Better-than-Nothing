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

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace BetterThanNothing
{
	struct TerrainFlattenZone
	{
		float centerX = 0.0f;
		float centerZ = 0.0f;
		float halfExtentX = 21.5f;
		float halfExtentZ = 14.5f;
		float targetHeight = -0.18f;
		float blendRadius = 12.0f;
	};

	struct TerrainWaterBasin
	{
		float centerX = 0.0f;
		float centerZ = 0.0f;
		float radiusX = 24.0f;
		float radiusZ = 20.0f;
		float depth = 4.2f;
	};

	struct TerrainLayerTexturePaths
	{
		std::string albedoPath;
		std::string normalPath;
		std::string roughnessPath;
	};

	struct TerrainConfig
	{
		uint32_t seed = 1337u;
		int32_t	 chunksX = 8;
		int32_t	 chunksZ = 8;
		float	 chunkSize = 32.0f;
		uint32_t chunkResolution = 64; // Quads per chunk axis (0.5m spacing at chunkSize = 32m)
		uint32_t waterResolution = 32; // Quads per water chunk axis (1.0m spacing for smooth Gerstner waves)

		float hillAmplitude = 11.5f;
		float mountainAmplitude = 20.0f;
		float baseFrequency = 0.014f;
		float	 waterLevel = -1.6f;
		float	 uvScale = 0.65f;
		uint32_t textureMaxDimension = 1024;
		uint32_t grassTuftsPerChunk = 320; // 3 curved 3D blades per tuft (~960 blades per grassy chunk)
		float	 grassMaxRadiusFromCenter = 96.0f;

		std::vector<TerrainFlattenZone> flattenZones;
		std::vector<TerrainWaterBasin>	waterBasins;

		// 0: Sand/Shore, 1: Mossy Grass, 2: Forest Dirt, 3: Mossy Rock, 4: Rocky Gravel
		std::array<TerrainLayerTexturePaths, 5> layerTextures{};

		static TerrainConfig CreateSponzaLandscapeDefaults()
		{
			TerrainConfig cfg{};
			cfg.seed = 4207u;
			cfg.chunksX = 8;
			cfg.chunksZ = 8;
			cfg.chunkSize = 32.0f;
			cfg.chunkResolution = 32; // 1.0m vertex spacing (4x lighter BVH & G-Buffer)
			cfg.waterResolution = 24;
			cfg.hillAmplitude = 9.5f;
			cfg.mountainAmplitude = 22.0f;
			cfg.baseFrequency = 0.015f;
			cfg.waterLevel = -1.6f;
			cfg.uvScale = 0.65f;
			cfg.textureMaxDimension = 1024;

			// Flat courtyard plateau underneath Sponza (Sponza bounds: X[-19.2, 18.0], Z[-11.8, 11.1], floor Y=0.0)
			cfg.flattenZones.push_back({
				.centerX = -0.6f,
				.centerZ = -0.4f,
				.halfExtentX = 21.5f,
				.halfExtentZ = 14.5f,
				.targetHeight = -0.18f,
				.blendRadius = 13.0f,
			});

			// Scenic lakes and ponds surrounding Sponza
			cfg.waterBasins.push_back({.centerX = 48.0f, .centerZ = -20.0f, .radiusX = 28.0f, .radiusZ = 22.0f, .depth = 4.8f});
			cfg.waterBasins.push_back({.centerX = -46.0f, .centerZ = 24.0f, .radiusX = 25.0f, .radiusZ = 21.0f, .depth = 4.4f});
			cfg.waterBasins.push_back({.centerX = -10.0f, .centerZ = -42.0f, .radiusX = 26.0f, .radiusZ = 16.0f, .depth = 4.0f});
			cfg.waterBasins.push_back({.centerX = 36.0f, .centerZ = 38.0f, .radiusX = 22.0f, .radiusZ = 19.0f, .depth = 3.9f});

			// 5 PBR texture packs from .assets/ (with Grass002 for lush green meadows)
			cfg.layerTextures[0] = {
				".assets/Ground049A_8K-JPG/Ground049A_8K-JPG_Color.jpg",
				".assets/Ground049A_8K-JPG/Ground049A_8K-JPG_NormalGL.jpg",
				".assets/Ground049A_8K-JPG/Ground049A_8K-JPG_Roughness.jpg",
			};
			cfg.layerTextures[1] = {
				".assets/Grass002_8K-JPG/Grass002_8K-JPG_Color.jpg",
				".assets/Grass002_8K-JPG/Grass002_8K-JPG_NormalGL.jpg",
				".assets/Grass002_8K-JPG/Grass002_8K-JPG_Roughness.jpg",
			};
			cfg.layerTextures[2] = {
				".assets/Ground071_8K-JPG/Ground071_8K-JPG_Color.jpg",
				".assets/Ground071_8K-JPG/Ground071_8K-JPG_NormalGL.jpg",
				".assets/Ground071_8K-JPG/Ground071_8K-JPG_Roughness.jpg",
			};
			cfg.layerTextures[3] = {
				".assets/Ground086_8K-JPG/Ground086_8K-JPG_Color.jpg",
				".assets/Ground086_8K-JPG/Ground086_8K-JPG_NormalGL.jpg",
				".assets/Ground086_8K-JPG/Ground086_8K-JPG_Roughness.jpg",
			};
			cfg.layerTextures[4] = {
				".assets/Ground108_8K-JPG/Ground108_8K-JPG_Color.jpg",
				".assets/Ground108_8K-JPG/Ground108_8K-JPG_NormalGL.jpg",
				".assets/Ground108_8K-JPG/Ground108_8K-JPG_Roughness.jpg",
			};

			return cfg;
		}
	};

	struct TreeScatterConfig
	{
		std::string modelPath = ".assets/tree_elm/scene.gltf";
		uint32_t	targetCount = 36;
		float		minSpacing = 14.0f;
		float		minScale = 0.48f;
		float		maxScale = 0.85f;
		float		maxSlope = 0.28f;
		float		minHeightAboveWater = 0.85f;
		float		worldMargin = 14.0f;
		uint32_t	seed = 9001u;

		// Axis-aligned exclusion box around Sponza
		float excludeMinX = -26.0f;
		float excludeMaxX = 25.0f;
		float excludeMinZ = -19.0f;
		float excludeMaxZ = 18.0f;
	};
}
