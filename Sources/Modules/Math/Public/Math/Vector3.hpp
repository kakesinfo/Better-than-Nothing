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
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <ostream>

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

		Vector3() : m_data(0.0f, 0.0f, 0.0f) {}
		Vector3(float x, float y, float z) : m_data(x, y, z) {}
		explicit Vector3(const glm::vec3& other) : m_data(other) {}
		explicit Vector3(float value) : m_data(value) {}

		// TODO: Add copy constructor and copy assignment operator
		Vector3(const Vector3& other) : m_data(other.m_data) {}

		[[nodiscard]] float GetX() const { return m_data.x; }
		[[nodiscard]] float GetY() const { return m_data.y; }
		[[nodiscard]] float GetZ() const { return m_data.z; }

		[[nodiscard]] float& x() { return m_data.x; }
		[[nodiscard]] float& y() { return m_data.y; }
		[[nodiscard]] float& z() { return m_data.z; }

		void SetX(float x) { m_data.x = x; }
		void SetY(float y) { m_data.y = y; }
		void SetZ(float z) { m_data.z = z; }

		[[nodiscard]] Vector3 operator+(const Vector3& other) const { return Vector3(m_data + other.m_data); }
		[[nodiscard]] Vector3 operator-(const Vector3& other) const { return Vector3(m_data - other.m_data); }
		[[nodiscard]] Vector3 operator*(float scalar) const { return Vector3(m_data * scalar); }
		[[nodiscard]] Vector3 operator/(float scalar) const { return Vector3(m_data / scalar); }
		[[nodiscard]] Vector3 operator-() const { return Vector3(-m_data); }

		Vector3 operator+=(const Vector3& other)
		{
			m_data += other.m_data;
			return *this;
		}

		Vector3 operator-=(const Vector3& other)
		{
			m_data -= other.m_data;
			return *this;
		}

		Vector3 operator*=(float scalar)
		{
			m_data *= scalar;
			return *this;
		}

		Vector3 operator/=(float scalar)
		{
			m_data /= scalar;
			return *this;
		}

		Vector3 operator=(const Vector3& other)
		{
			m_data = other.m_data;
			return *this;
		}

		[[nodiscard]] bool Equals(const Vector3& other) const { return m_data == other.m_data; }

		[[nodiscard]] float Magnitude() const { return glm::length(m_data); }

		[[nodiscard]] Vector3 Normalized() const { return Vector3(glm::normalize(m_data)); }

		[[nodiscard]] glm::vec3 Value() const { return m_data; }

		static float   Dot(const Vector3& a, const Vector3& b) { return glm::dot(a.m_data, b.m_data); }
		static float   Distance(const Vector3& a, const Vector3& b) { return glm::distance(a.m_data, b.m_data); }
		static float   Distance2(const Vector3& a, const Vector3& b) { return glm::distance2(a.m_data, b.m_data); }
		static Vector3 Lerp(const Vector3& a, const Vector3& b, float t) { return Vector3(glm::mix(a.m_data, b.m_data, t)); }
		static Vector3 Max(const Vector3& a, const Vector3& b) { return Vector3(glm::max(a.m_data, b.m_data)); }
		static Vector3 Min(const Vector3& a, const Vector3& b) { return Vector3(glm::min(a.m_data, b.m_data)); }

		friend std::ostream& operator<<(std::ostream& os, const Vector3& vec)
		{
			os << "Vector3(" << vec.GetX() << ", " << vec.GetY() << ")";
			return os;
		}

	private:
		glm::vec3 m_data;
	};
}
