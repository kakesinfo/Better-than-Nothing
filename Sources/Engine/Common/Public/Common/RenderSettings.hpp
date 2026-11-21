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

#include <cstdint>

namespace BetterThanNothing
{
	enum class EDebugViewMode : uint32_t
	{
		Final = 0,
		Albedo = 1,
		Normal = 2,
		Metallic = 3,
		Roughness = 4,
		MotionVectors = 5,
		RTShadows = 6,
		AmbientOcclusion = 7,
		DiffuseIrradiance = 8,
		Depth = 9,
		ScreenSpaceShadows = 10
	};

	enum class EToneMapMode : uint32_t
	{
		AgX = 0,
		ACES = 1,
		Reinhard = 2
	};

	enum class EAOMode : uint32_t
	{
		Off = 0,
		GTAO = 1,
		RTAO = 2
	};

	struct RenderSettingsData
	{
		EDebugViewMode debugView = EDebugViewMode::Final;
		EToneMapMode   toneMapMode = EToneMapMode::AgX;

		// RT Shadows + AMD FidelityFX Shadow Denoiser
		bool  enableRTShadows = true;
		bool  enableShadowDenoiser = true;
		float sunAngularRadius = 0.015f;

		// Bend Studio Screen Space Shadows
		bool	 enableBendSSS = true;
		float	 sssSurfaceThickness = 0.005f;
		float	 sssBilinearThreshold = 0.02f;
		float	 sssShadowContrast = 4.0f;
		bool	 sssIgnoreEdgePixels = false;
		bool	 sssBilinearOffsetMode = false;
		bool	 sssUseEarlyOut = true;
		uint32_t sssDebugMode = 0; // 0 = None, 1 = Edge Mask, 2 = Thread Index, 3 = Wave Index

		// Ambient Occlusion (GTAO / RTAO)
		EAOMode aoMode = EAOMode::RTAO; // Automatically falls back to GTAO if HW RT unavailable
		float	aoRadius = 1.5f;
		float	aoPower = 1.4f;

		// Irradiance / IBL
		float irradianceIntensity = 1.0f;

		// Hillaire (EGSR 2020) Sky & Atmosphere parameters
		float skyLuminanceNits = 4000.0f;
		float atmosphereAltitudeKm = 0.2f;
		float atmosphereMieAnisotropy = 0.8f;
		float atmosphereMultiScatteringFactor = 1.0f;
		float atmosphereDensityScale = 1.0f;

		// Temporal Anti-Aliasing
		bool  enableTAA = true;
		float taaBlendFactor = 0.1f;

		bool rtSupported = false;
	};

	class RenderSettings
	{
	public:
		static RenderSettingsData& Get()
		{
			static RenderSettingsData s_settings{};
			return s_settings;
		}
	};
}
