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
#include <string>
#include <vector>

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
		ScreenSpaceShadows = 10,
		VolumetricFog = 11
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

	enum class EUpscalerMode : uint32_t
	{
		Off = 0,
		TAA = 1,
		FSR = 2 // AMD FidelityFX SDK v2.4.0 Upscaler (FSR 4.1.2 / 3.1.6)
	};

	enum class EFsrQualityMode : uint32_t
	{
		NativeAA = 0,		  // 1.0x (FFX_UPSCALE_QUALITY_MODE_NATIVEAA)
		Quality = 1,		  // 1.5x (FFX_UPSCALE_QUALITY_MODE_QUALITY)
		Balanced = 2,		  // 1.7x (FFX_UPSCALE_QUALITY_MODE_BALANCED)
		Performance = 3,	  // 2.0x (FFX_UPSCALE_QUALITY_MODE_PERFORMANCE)
		UltraPerformance = 4  // 3.0x (FFX_UPSCALE_QUALITY_MODE_ULTRA_PERFORMANCE)
	};

	struct RenderSettingsData
	{
		EDebugViewMode debugView = EDebugViewMode::Final;
		EToneMapMode   toneMapMode = EToneMapMode::AgX;

		// RT Shadows + AMD FidelityFX Shadow Denoiser (SDK v2.4.0 / FFX_DNSR_Shadows)
		bool  enableRTShadows = true;
		bool  enableShadowDenoiser = true;
		float sunAngularRadius = 0.015f;
		// FidelityFX Denoiser configurable filter parameters (mapped from FfxApiConfigureDenoiserKey & A-Trous weights)
		float denoiserDepthSigma = 0.015f;			  // Disocclusion / bilateral depth sigma
		float denoiserNormalSigma = 16.0f;			  // Cross-bilateral normal strength
		float denoiserShadowPhi = 4.0f;				  // Variance-guided edge-stopping relaxation
		float denoiserTemporalStabilityBias = 0.08f;  // Minimum temporal blend alpha (FFX_API_CONFIGURE_DENOISER_KEY_STABILITY_BIAS)
		float denoiserRadianceClipStdK = 1.5f;		  // Neighborhood std-dev clamping K (FFX_API_CONFIGURE_DENOISER_KEY_RADIANCE_CLIP_STD_K)
		float denoiserDisocclusionThreshold = 0.025f; // Relative depth disocclusion threshold (FFX_API_CONFIGURE_DENOISER_KEY_DISOCCLUSION_THRESHOLD)

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

		// Volumetric Clouds & Cloud Shadows
		bool  enableVolumetricClouds = true;
		float cloudCoverage = 0.52f;
		float cloudDensity = 1.0f;
		float cloudSpeed = 1.0f;
		float cloudShadowStrength = 0.65f;

		// Upscaling & Temporal Anti-Aliasing (AMD FidelityFX FSR 4.1.2 / Native TAA)
		EUpscalerMode	upscalerMode = EUpscalerMode::FSR;
		EFsrQualityMode fsrQualityMode = EFsrQualityMode::NativeAA;
		bool			fsrEnableSharpening = true;
		float			fsrSharpness = 0.70f;
		bool			fsrDrawDebugView = false;
		float			fsrVelocityFactor = 1.0f; // FFX_API_CONFIGURE_UPSCALE_KEY_FVELOCITYFACTOR
		uint32_t		fsrSelectedVersionIndex = 0;

		// Read-only runtime telemetry populated by FidelityFX SDK v2.4.0
		bool					 fsrAvailable = false;
		std::string				 fsrActiveVersionName = "4.1.2";
		uint64_t				 fsrActiveVersionId = 0;
		float					 fsrGpuMemoryMb = 0.0f;
		uint32_t				 fsrRenderWidth = 0;
		uint32_t				 fsrRenderHeight = 0;
		uint32_t				 fsrDisplayWidth = 0;
		uint32_t				 fsrDisplayHeight = 0;
		std::vector<std::string> fsrVersionNames;

		bool					 ffxDenoiserSdkHeaderPresent = true;
		std::string				 ffxDenoiserSdkVersionName = "1.2.1 (FFX_DNSR_Shadows Vulkan + Ray Regeneration 1.2.1 SDK)";

		// Legacy TAA controls (kept synchronized with upscalerMode)
		bool  enableTAA = true;
		float taaBlendFactor = 0.1f;

		// Performance & Frame Pacing
		uint32_t targetFps = 0; // 0 = VSync refresh rate only (avoids CPU sleep vs VBlank phase oscillation)

		// Water rendering
		float waterLevel = -1.6f;
		float waterWaveScale = 1.0f;
		float waterFoamIntensity = 1.0f;

		// Procedural 3D Grass
		bool  enableProceduralGrass = true;
		float grassWindStrength = 1.0f;
		float grassWindSpeed = 1.0f;
		float grassFadeDistance = 145.0f;

		// Volumetric Raymarched Fog (WsfBDf) & Canopy/Relief Volumetric Shadows
		bool	 enableVolumetricFog = true;
		bool	 fogRestrictToSponza = true;
		bool	 enableVolumetricShadows = true; // Volumetric sun/shadow shafts from tree canopies & hill reliefs
		float	 volumetricShadowIntensity = 1.0f;
		bool	 fogUseBlueNoise = true;
		bool	 fogUseRTShadows = true;
		float	 fogDensity = 0.022f;
		float	 fogHeightFalloff = 0.14f;
		float	 fogBaseHeight = 0.0f;
		float	 fogMaxDistance = 135.0f;
		float	 fogAnisotropy = 0.68f;
		float	 fogNoiseStrength = 0.55f;
		uint32_t fogStepCount = 16;

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
