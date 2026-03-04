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
	class Vector4
	{
	public:
		static const Vector4 One;
		static const Vector4 Zero;

		Vector4() : m_data(0.0f, 0.0f, 0.0f, 0.0f) {}
		Vector4(float x, float y, float z, float w) : m_data(x, y, z, w) {}
		explicit Vector4(const glm::vec4& other) : m_data(other) {}
		explicit Vector4(float value) : m_data(value) {}

		// TODO: Add copy constructor and copy assignment operator
		Vector4(const Vector4& other) : m_data(other.m_data) {}

		[[nodiscard]] float GetW() const { return m_data.w; }
		[[nodiscard]] float GetX() const { return m_data.x; }
		[[nodiscard]] float GetY() const { return m_data.y; }
		[[nodiscard]] float GetZ() const { return m_data.z; }

		void SetW(float w) { m_data.w = w; }
		void SetX(float x) { m_data.x = x; }
		void SetY(float y) { m_data.y = y; }
		void SetZ(float z) { m_data.z = z; }

		[[nodiscard]] Vector4 operator+(const Vector4& other) const { return Vector4(m_data + other.m_data); }
		[[nodiscard]] Vector4 operator-(const Vector4& other) const { return Vector4(m_data - other.m_data); }
		[[nodiscard]] Vector4 operator*(float scalar) const { return Vector4(m_data * scalar); }
		[[nodiscard]] Vector4 operator/(float scalar) const { return Vector4(m_data / scalar); }
		[[nodiscard]] Vector4 operator-() const { return Vector4(-m_data); }

		Vector4 operator+=(const Vector4& other)
		{
			m_data += other.m_data;
			return *this;
		}

		Vector4 operator-=(const Vector4& other)
		{
			m_data -= other.m_data;
			return *this;
		}

		Vector4 operator*=(float scalar)
		{
			m_data *= scalar;
			return *this;
		}

		Vector4 operator/=(float scalar)
		{
			m_data /= scalar;
			return *this;
		}

		Vector4 operator=(const Vector4& other)
		{
			m_data = other.m_data;
			return *this;
		}

		[[nodiscard]] bool Equals(const Vector4& other) const { return m_data == other.m_data; }

		[[nodiscard]] float Magnitude() const { return glm::length(m_data); }

		[[nodiscard]] Vector4 Normalized() const { return Vector4(glm::normalize(m_data)); }

		[[nodiscard]] glm::vec3 Value() const { return m_data; }

		static float   Dot(const Vector4& a, const Vector4& b) { return glm::dot(a.m_data, b.m_data); }
		static float   Distance(const Vector4& a, const Vector4& b) { return glm::distance(a.m_data, b.m_data); }
		static Vector4 Max(const Vector4& a, const Vector4& b) { return Vector4(glm::max(a.m_data, b.m_data)); }
		static Vector4 Min(const Vector4& a, const Vector4& b) { return Vector4(glm::min(a.m_data, b.m_data)); }

		friend std::ostream& operator<<(std::ostream& os, const Vector4& vec)
		{
			os << "Vector4(" << vec.GetX() << ", " << vec.GetY() << ")";
			return os;
		}

	private:
		glm::vec4 m_data;
	};
}
