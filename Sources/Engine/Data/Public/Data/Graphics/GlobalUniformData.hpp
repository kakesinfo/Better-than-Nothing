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

#include "Math/Matrix4.hpp"
#include "Math/Vector3.hpp"
#include <cstddef>
#include <cstdint>

namespace BetterThanNothing
{
	namespace GlobalRenderFlags
	{
		inline constexpr uint32_t RTShadowsEnabled = 1u << 0;
		inline constexpr uint32_t ShadowDenoiserEnabled = 1u << 1;
		inline constexpr uint32_t TAAEnabled = 1u << 2;
		inline constexpr uint32_t HistoryValid = 1u << 3;
		inline constexpr uint32_t AOEnabled = 1u << 4;
		inline constexpr uint32_t BendSSSEnabled = 1u << 5;
		inline constexpr uint32_t VolumetricCloudsEnabled = 1u << 6;
		inline constexpr uint32_t SunMoved = 1u << 7;
	}

	struct GlobalUniformData
	{
		alignas(16) Matrix4 projection;
		alignas(16) Matrix4 view;
		alignas(16) Matrix4 viewProj;
		alignas(16) Matrix4 invViewProj;
		alignas(16) Matrix4 unjitteredViewProj;
		alignas(16) Matrix4 prevUnjitteredViewProj;

		alignas(16) Vector3 lightDirection;
		float sunAngularRadius = 0.015f;

		alignas(16) Vector3 lightColor;
		float sunIlluminanceLux = 115000.0f;

		alignas(16) Vector3 cameraPosition;
		float cameraExposure = 1.0f;

		uint32_t pointLightCount = 0;
		uint32_t frameIndex = 0;
		uint32_t screenWidth = 1;
		uint32_t screenHeight = 1;

		uint32_t debugView = 0;
		uint32_t toneMapMode = 0;
		uint32_t flags = 0;
		float	 taaBlendFactor = 0.1f;

		float skyLuminanceNits = 4000.0f;
		float atmosphereAltitudeKm = 0.2f;
		float atmosphereMieAnisotropy = 0.8f;
		float atmosphereMultiScatteringFactor = 1.0f;

		float atmosphereDensityScale = 1.0f;
		float aoRadius = 1.5f;
		float aoPower = 1.4f;
		float irradianceIntensity = 1.0f;

		float time = 0.0f;
		float waterLevel = -1.6f;
		float waterWaveScale = 1.0f;
		float waterFoamIntensity = 1.0f;

		float grassWindStrength = 1.0f;
		float grassWindSpeed = 1.0f;
		float grassFadeDistance = 85.0f;
		float deltaTime = 0.0166f;

		float cloudCoverage = 0.52f;
		float cloudDensity = 1.0f;
		float cloudSpeed = 1.0f;
		float cloudShadowStrength = 0.65f;
	};

	static_assert(offsetof(GlobalUniformData, projection) == 0);
	static_assert(offsetof(GlobalUniformData, view) == 64);
	static_assert(offsetof(GlobalUniformData, viewProj) == 128);
	static_assert(offsetof(GlobalUniformData, invViewProj) == 192);
	static_assert(offsetof(GlobalUniformData, unjitteredViewProj) == 256);
	static_assert(offsetof(GlobalUniformData, prevUnjitteredViewProj) == 320);
	static_assert(offsetof(GlobalUniformData, lightDirection) == 384);
	static_assert(offsetof(GlobalUniformData, sunAngularRadius) == 396);
	static_assert(offsetof(GlobalUniformData, lightColor) == 400);
	static_assert(offsetof(GlobalUniformData, sunIlluminanceLux) == 412);
	static_assert(offsetof(GlobalUniformData, cameraPosition) == 416);
	static_assert(offsetof(GlobalUniformData, cameraExposure) == 428);
	static_assert(offsetof(GlobalUniformData, pointLightCount) == 432);
	static_assert(offsetof(GlobalUniformData, frameIndex) == 436);
	static_assert(offsetof(GlobalUniformData, screenWidth) == 440);
	static_assert(offsetof(GlobalUniformData, screenHeight) == 444);
	static_assert(offsetof(GlobalUniformData, debugView) == 448);
	static_assert(offsetof(GlobalUniformData, toneMapMode) == 452);
	static_assert(offsetof(GlobalUniformData, flags) == 456);
	static_assert(offsetof(GlobalUniformData, taaBlendFactor) == 460);
	static_assert(offsetof(GlobalUniformData, skyLuminanceNits) == 464);
	static_assert(offsetof(GlobalUniformData, atmosphereAltitudeKm) == 468);
	static_assert(offsetof(GlobalUniformData, atmosphereMieAnisotropy) == 472);
	static_assert(offsetof(GlobalUniformData, atmosphereMultiScatteringFactor) == 476);
	static_assert(offsetof(GlobalUniformData, atmosphereDensityScale) == 480);
	static_assert(offsetof(GlobalUniformData, aoRadius) == 484);
	static_assert(offsetof(GlobalUniformData, aoPower) == 488);
	static_assert(offsetof(GlobalUniformData, irradianceIntensity) == 492);
	static_assert(offsetof(GlobalUniformData, time) == 496);
	static_assert(offsetof(GlobalUniformData, waterLevel) == 500);
	static_assert(offsetof(GlobalUniformData, waterWaveScale) == 504);
	static_assert(offsetof(GlobalUniformData, waterFoamIntensity) == 508);
	static_assert(offsetof(GlobalUniformData, grassWindStrength) == 512);
	static_assert(offsetof(GlobalUniformData, grassWindSpeed) == 516);
	static_assert(offsetof(GlobalUniformData, grassFadeDistance) == 520);
	static_assert(offsetof(GlobalUniformData, deltaTime) == 524);
	static_assert(offsetof(GlobalUniformData, cloudCoverage) == 528);
	static_assert(offsetof(GlobalUniformData, cloudDensity) == 532);
	static_assert(offsetof(GlobalUniformData, cloudSpeed) == 536);
	static_assert(offsetof(GlobalUniformData, cloudShadowStrength) == 540);
	static_assert(sizeof(GlobalUniformData) == 544);
}
