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

#include "Math/Vector3.hpp"
#include <cstddef>

namespace BetterThanNothing
{
	struct PointLightData
	{
		alignas(16) Vector3 position;
		alignas(16) Vector3 color;

		float constant = 1.0f;
		float linear = 0.0f;
		float quadratic = 0.0f;
		float intensity = 0.0f;
	};

	static_assert(offsetof(PointLightData, position) == 0);
	static_assert(offsetof(PointLightData, color) == 16);
	static_assert(offsetof(PointLightData, constant) == 28);
	static_assert(offsetof(PointLightData, linear) == 32);
	static_assert(offsetof(PointLightData, quadratic) == 36);
	static_assert(offsetof(PointLightData, intensity) == 40);
	static_assert(sizeof(PointLightData) == 48);
}
