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
		float radius = 15.0f; // Attenuation cutoff radius in meters (m)

		alignas(16) Vector3 color;
		float luminousPowerLm = 1600.0f; // Luminous flux in lumens (lm)
	};

	static_assert(offsetof(PointLightData, position) == 0);
	static_assert(offsetof(PointLightData, radius) == 12);
	static_assert(offsetof(PointLightData, color) == 16);
	static_assert(offsetof(PointLightData, luminousPowerLm) == 28);
	static_assert(sizeof(PointLightData) == 32);
}
