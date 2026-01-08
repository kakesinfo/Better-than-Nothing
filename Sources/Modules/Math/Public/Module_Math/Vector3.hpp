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
	class Vector3
	{
	public:
		static const Vector3 Down;
		static const Vector3 Left;
		static const Vector3 One;
		static const Vector3 Right;
		static const Vector3 Up;
		static const Vector3 Forward;
		static const Vector3 Zero;

		Vector3();
		Vector3(float x, float y, float z);

		explicit Vector3(const glm::vec3& other);
		explicit Vector3(float value);

		// TODO: Add copy constructor and copy assignment operator
		Vector3(const Vector3& other);

		[[nodiscard]] float GetX() const;
		[[nodiscard]] float GetY() const;
		[[nodiscard]] float GetZ() const;

		void SetX(float x);
		void SetY(float y);
		void SetZ(float z);

		[[nodiscard]] Vector3 operator+(const Vector3& other) const;
		[[nodiscard]] Vector3 operator-(const Vector3& other) const;
		[[nodiscard]] Vector3 operator*(float scalar) const;
		[[nodiscard]] Vector3 operator/(float scalar) const;
		[[nodiscard]] Vector3 operator-() const;

		Vector3 operator+=(const Vector3& other);
		Vector3 operator-=(const Vector3& other);
		Vector3 operator*=(float scalar);
		Vector3 operator/=(float scalar);
		Vector3 operator=(const Vector3& other);

		[[nodiscard]] bool		Equals(const Vector3& other) const;
		[[nodiscard]] float		Magnitude() const;
		[[nodiscard]] Vector3	Normalized() const;
		[[nodiscard]] glm::vec3 Value() const;

		static float   Dot(const Vector3& a, const Vector3& b);
		static float   Distance(const Vector3& a, const Vector3& b);
		static Vector3 Lerp(const Vector3& a, const Vector3& b, float t);
		static Vector3 Max(const Vector3& a, const Vector3& b);
		static Vector3 Min(const Vector3& a, const Vector3& b);

		friend std::ostream& operator<<(std::ostream& os, const Vector3& vec);

	private:
		glm::vec3 m_data;
	};
}
