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

#include "module_math.hpp"

namespace BetterThanNothing
{
	class Vector3;

	class Quaternion
	{
	public:
		static const Quaternion Identity;

		Quaternion();
		Quaternion(float w, float x, float y, float z);
		explicit Quaternion(const glm::quat& quat);

		float GetX() const;
		float GetY() const;
		float GetZ() const;
		float GetW() const;

		[[nodiscard]] Quaternion operator*(const Quaternion& other) const;
		[[nodiscard]] Quaternion operator+(const Quaternion& other) const;
		[[nodiscard]] Quaternion operator-(const Quaternion& other) const;
		[[nodiscard]] Vector3 operator*(const Vector3& other) const;

		static Quaternion AngleAxis(float angleInDegrees, float x, float y, float z);
		static Quaternion AngleAxis(float angleInDegrees, const Vector3& axis);

		[[nodiscard]] Quaternion Normalized() const;
		[[nodiscard]] Quaternion Conjugated() const;

		[[nodiscard]] float		Magnitude() const;
		[[nodiscard]] bool		Equals(const Quaternion& other) const;
		[[nodiscard]] glm::quat Value() const;

		friend std::ostream& operator<<(std::ostream& os, const Quaternion& vec);

	private:
		glm::quat m_data;
	};
}