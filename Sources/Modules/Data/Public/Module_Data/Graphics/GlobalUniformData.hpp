/*
 *	Better than Nothing
 *	Copyright (C) 2025 kakesinfo
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

#include "Module_Math/Matrix4.hpp"
#include "Module_Math/Vector3.hpp"

namespace BetterThanNothing
{
	struct GlobalUniformData
	{
		alignas(16) Matrix4 projection;
		alignas(16) Matrix4 view;

		alignas(16) Vector3 lightDirection;
		alignas(16) Vector3 lightColor;
		float lightIntensity;

		alignas(16) Vector3 cameraPosition;
		float cameraExposure;
		int pointLightCount;
	};
}