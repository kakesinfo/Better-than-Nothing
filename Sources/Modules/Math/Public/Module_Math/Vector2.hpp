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
	class Vector2
	{
	public:
		static const Vector2 Down;
		static const Vector2 Left;
		static const Vector2 One;
		static const Vector2 Right;
		static const Vector2 Up;
		static const Vector2 Zero;

		Vector2();
		Vector2(float x, float y);
		explicit Vector2(float value);
		explicit Vector2(const glm::vec2& other);

		// TODO: Add copy constructor and copy assignment operator
		Vector2(const Vector2& other);

		[[nodiscard]] float GetX() const;
		[[nodiscard]] float GetY() const;

		void SetX(float x);
		void SetY(float y);

		[[nodiscard]] Vector2 operator+(const Vector2& other) const;
		[[nodiscard]] Vector2 operator-(const Vector2& other) const;
		[[nodiscard]] Vector2 operator*(float scalar) const;
		[[nodiscard]] Vector2 operator/(float scalar) const;
		[[nodiscard]] Vector2 operator-() const;

		Vector2 operator+=(const Vector2& other);
		Vector2 operator-=(const Vector2& other);
		Vector2 operator*=(float scalar);
		Vector2 operator/=(float scalar);
		Vector2 operator=(const Vector2& other);

		[[nodiscard]] bool		Equals(const Vector2& other) const;
		[[nodiscard]] float		Magnitude() const;
		[[nodiscard]] Vector2	Normalized() const;
		[[nodiscard]] glm::vec2 Value() const;

		static float   Dot(const Vector2& a, const Vector2& b);
		static float   Distance(const Vector2& a, const Vector2& b);
		static Vector2 Lerp(const Vector2& a, const Vector2& b, float t);
		static Vector2 Max(const Vector2& a, const Vector2& b);
		static Vector2 Min(const Vector2& a, const Vector2& b);

		friend std::ostream& operator<<(std::ostream& os, const Vector2& vec);

	private:
		glm::vec2 m_data;
	};
}