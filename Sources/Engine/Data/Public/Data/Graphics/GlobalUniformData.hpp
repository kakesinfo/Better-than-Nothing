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
	struct GlobalUniformData
	{
		alignas(16) Matrix4 projection;
		alignas(16) Matrix4 view;
		alignas(16) Matrix4 viewProj;
		alignas(16) Matrix4 invViewProj;
		alignas(16) Matrix4 unjitteredViewProj;
		alignas(16) Matrix4 prevUnjitteredViewProj;

		alignas(16) Vector3 lightDirection;
		float lightIntensity = 0.0f;

		alignas(16) Vector3 lightColor;
		float cameraExposure = 1.0f;

		alignas(16) Vector3 cameraPosition;
		uint32_t pointLightCount = 0;

		float	 screenWidth = 1280.0f;
		float	 screenHeight = 720.0f;
		uint32_t frameIndex = 0;
		uint32_t debugView = 0;

		uint32_t toneMapMode = 0;
		uint32_t flags = 0;
		float	 elapsedTime = 0.0f;
		float	 deltaTime = 0.0f;

		float irradianceIntensity = 1.0f;
		float waterWaveScale = 1.0f;
		float waterFoamIntensity = 1.0f;
		float grassWindStrength = 1.0f;

		float grassWindSpeed = 1.0f;
		float grassFadeDistance = 145.0f;
		float cloudDensity = 1.0f;
		float cloudShadowStrength = 0.65f;
	};

	static_assert(offsetof(GlobalUniformData, projection) == 0);
	static_assert(offsetof(GlobalUniformData, view) == 64);
	static_assert(offsetof(GlobalUniformData, viewProj) == 128);
	static_assert(offsetof(GlobalUniformData, invViewProj) == 192);
	static_assert(offsetof(GlobalUniformData, unjitteredViewProj) == 256);
	static_assert(offsetof(GlobalUniformData, prevUnjitteredViewProj) == 320);
	static_assert(offsetof(GlobalUniformData, lightDirection) == 384);
	static_assert(offsetof(GlobalUniformData, lightIntensity) == 396);
	static_assert(offsetof(GlobalUniformData, lightColor) == 400);
	static_assert(offsetof(GlobalUniformData, cameraExposure) == 412);
	static_assert(offsetof(GlobalUniformData, cameraPosition) == 416);
	static_assert(offsetof(GlobalUniformData, pointLightCount) == 428);
	static_assert(offsetof(GlobalUniformData, screenWidth) == 432);
	static_assert(offsetof(GlobalUniformData, screenHeight) == 436);
	static_assert(offsetof(GlobalUniformData, frameIndex) == 440);
	static_assert(offsetof(GlobalUniformData, debugView) == 444);
	static_assert(offsetof(GlobalUniformData, toneMapMode) == 448);
	static_assert(offsetof(GlobalUniformData, flags) == 452);
	static_assert(offsetof(GlobalUniformData, elapsedTime) == 456);
	static_assert(offsetof(GlobalUniformData, deltaTime) == 460);
	static_assert(offsetof(GlobalUniformData, irradianceIntensity) == 464);
	static_assert(offsetof(GlobalUniformData, waterWaveScale) == 468);
	static_assert(offsetof(GlobalUniformData, waterFoamIntensity) == 472);
	static_assert(offsetof(GlobalUniformData, grassWindStrength) == 476);
	static_assert(offsetof(GlobalUniformData, grassWindSpeed) == 480);
	static_assert(offsetof(GlobalUniformData, grassFadeDistance) == 484);
	static_assert(offsetof(GlobalUniformData, cloudDensity) == 488);
	static_assert(offsetof(GlobalUniformData, cloudShadowStrength) == 492);
	static_assert(sizeof(GlobalUniformData) == 496);
}
