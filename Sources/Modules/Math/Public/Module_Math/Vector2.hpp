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

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <ostream>

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

		Vector2() : m_data(0.0f, 0.0f) {}
		Vector2(float x, float y) : m_data(x, y) {}
		explicit Vector2(const glm::vec2& other) : m_data(other) {}
		explicit Vector2(float value) : m_data(value) {}

		// TODO: Add copy constructor and copy assignment operator
		Vector2(const Vector2& other) : m_data(other.m_data) {}

		[[nodiscard]] float GetX() const { return m_data.x; }
		[[nodiscard]] float GetY() const { return m_data.y; }

		[[nodiscard]] float& x() { return m_data.x; }
		[[nodiscard]] float& y() { return m_data.y; }

		void SetX(float x) { m_data.x = x; }
		void SetY(float y) { m_data.y = y; }

		[[nodiscard]] Vector2 operator+(const Vector2& other) const { return Vector2(m_data + other.m_data); }
		[[nodiscard]] Vector2 operator-(const Vector2& other) const { return Vector2(m_data - other.m_data); }
		[[nodiscard]] Vector2 operator*(float scalar) const { return Vector2(m_data * scalar); }
		[[nodiscard]] Vector2 operator/(float scalar) const { return Vector2(m_data / scalar); }
		[[nodiscard]] Vector2 operator-() const { return Vector2(-m_data); }

		Vector2 operator+=(const Vector2& other)
		{
			m_data += other.m_data;
			return *this;
		}

		Vector2 operator-=(const Vector2& other)
		{
			m_data -= other.m_data;
			return *this;
		}

		Vector2 operator*=(float scalar)
		{
			m_data *= scalar;
			return *this;
		}

		Vector2 operator/=(float scalar)
		{
			m_data /= scalar;
			return *this;
		}

		Vector2& operator=(const Vector2& other)
		{
			m_data = other.m_data;
			return *this;
		}

		[[nodiscard]] bool		Equals(const Vector2& other) const { return m_data == other.m_data; }
		[[nodiscard]] float		Magnitude() const { return glm::length(m_data); }
		[[nodiscard]] Vector2	Normalized() const { return Vector2(glm::normalize(m_data)); }
		[[nodiscard]] glm::vec2 Value() const { return m_data; }

		static float   Dot(const Vector2& a, const Vector2& b) { return glm::dot(a.m_data, b.m_data); }
		static Vector2 Lerp(const Vector2& a, const Vector2& b, float t) { return Vector2(glm::mix(a.m_data, b.m_data, t)); }
		static Vector2 Min(const Vector2& a, const Vector2& b) { return Vector2(glm::min(a.m_data, b.m_data)); }
		static Vector2 Max(const Vector2& a, const Vector2& b) { return Vector2(glm::max(a.m_data, b.m_data)); }
		static float   Distance(const Vector2& a, const Vector2& b) { return glm::distance(a.m_data, b.m_data); }

		friend std::ostream& operator<<(std::ostream& os, const Vector2& vec)
		{
			os << "Vector2(" << vec.GetX() << ", " << vec.GetY() << ")";
			return os;
		}

	private:
		glm::vec2 m_data;
	};
}
