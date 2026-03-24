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

#include "Math/Vector3.hpp"

namespace BetterThanNothing
{
	const Vector3 Vector3::Up = Vector3(0.0f, 1.0f, 0.0f);
	const Vector3 Vector3::Down = Vector3(0.0f, -1.0f, 0.0f);
	const Vector3 Vector3::Left = Vector3(-1.0f, 0.0f, 0.0f);
	const Vector3 Vector3::Right = Vector3(1.0f, 0.0f, 0.0f);
	const Vector3 Vector3::Forward = Vector3(0.0f, 0.0f, -1.0f);
	const Vector3 Vector3::One = Vector3(1.0f, 1.0f, 1.0f);
	const Vector3 Vector3::Zero = Vector3(0.0f, 0.0f, 0.0f);
}
