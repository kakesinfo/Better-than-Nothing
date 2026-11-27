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

#include <chrono>
#include <cstdint>

namespace BetterThanNothing
{
	enum class EDebugViewMode : uint32_t
	{
		Final = 0,
		Albedo = 1,
		WorldNormal = 2,
		Metallic = 3,
		Roughness = 4,
		MotionVectors = 5,
		RTShadows = 6,
		AmbientOcclusion = 7,
		DiffuseIrradiance = 8,
		Depth = 9,
		ScreenSpaceShadows = 10,
		CloudTransmittance = 11
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
		EAOMode		   aoMode = EAOMode::RTAO;

		bool rtSupported = false;

		bool enableTAA = true;
		float taaBlendFactor = 0.1f;

		bool enableRTShadows = true;
		bool enableScreenSpaceShadows = true;
		bool enableClouds = true;

		float aoRadius = 1.5f;
		float aoStrength = 1.4f;

		float sunAngularRadius = 0.015f;
		float skyLuminanceNits = 4000.0f;
		float atmosphereAltitudeKm = 0.2f;
		float atmosphereMieAnisotropy = 0.8f;
		float atmosphereMultiScatteringFactor = 1.0f;
		float atmosphereDensityScale = 1.0f;
		float irradianceIntensity = 1.0f;

		float cloudCoverage = 0.52f;
		float cloudDensity = 1.0f;
		float cloudSpeed = 1.0f;
		float cloudShadowStrength = 0.65f;

		bool  enableProceduralGrass = true;
		float grassWindStrength = 1.0f;
		float grassWindSpeed = 1.0f;
		float grassFadeDistance = 85.0f;

		float waterLevel = -1.6f;
		float waterWaveScale = 1.0f;
		float waterFoamIntensity = 1.0f;

		int targetFps = 60;
		int sssDebugMode = 0;

		float sssSurfaceThickness = 0.005f;
		float sssBilinearThreshold = 0.02f;
		float sssShadowContrast = 2.0f;
		bool  sssIgnoreEdgePixels = false;
		bool  sssBilinearOffsetMode = false;
		bool  sssUseEarlyOut = true;
	};

	class RenderSettings
	{
	public:
		[[nodiscard]] static RenderSettingsData& Get()
		{
			static RenderSettingsData instance{};
			return instance;
		}

		[[nodiscard]] static std::chrono::microseconds ComputeFrameSleepDuration(const std::chrono::microseconds elapsed, const int targetFps) noexcept
		{
			if (targetFps <= 0)
				return std::chrono::microseconds{0};

			const auto targetFrameTime = std::chrono::microseconds(1000000 / targetFps);
			if (elapsed >= targetFrameTime)
				return std::chrono::microseconds{0};

			return targetFrameTime - elapsed;
		}
	};
}
