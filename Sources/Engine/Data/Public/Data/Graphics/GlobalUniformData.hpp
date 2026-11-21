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

		alignas(16) Vector3 lightDirection;
		alignas(16) Vector3 lightColor;
		float lightIntensity = 0.0f;

		alignas(16) Vector3 cameraPosition;
		float cameraExposure = 1.0f;
		uint32_t pointLightCount = 0;
	};

	static_assert(offsetof(GlobalUniformData, projection) == 0);
	static_assert(offsetof(GlobalUniformData, view) == 64);
	static_assert(offsetof(GlobalUniformData, lightDirection) == 128);
	static_assert(offsetof(GlobalUniformData, lightColor) == 144);
	static_assert(offsetof(GlobalUniformData, lightIntensity) == 156);
	static_assert(offsetof(GlobalUniformData, cameraPosition) == 160);
	static_assert(offsetof(GlobalUniformData, cameraExposure) == 172);
	static_assert(offsetof(GlobalUniformData, pointLightCount) == 176);
	static_assert(sizeof(GlobalUniformData) == 192);
}
