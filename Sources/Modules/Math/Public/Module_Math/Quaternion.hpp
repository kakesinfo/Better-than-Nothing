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

#include "Vector3.hpp"

#include <ostream>

namespace BetterThanNothing
{
	class Quaternion
	{
	public:
		static const Quaternion Identity;

		Quaternion() : m_data(0.0f, 0.0f, 0.0f, 0.0f) {}
		Quaternion(float w, float x, float y, float z) : m_data(w, x, y, z) {}
		explicit Quaternion(const glm::quat& quat) : m_data(quat) {}

		[[nodiscard]] float GetX() const { return m_data.x; }
		[[nodiscard]] float GetY() const { return m_data.y; }
		[[nodiscard]] float GetZ() const { return m_data.z; }
		[[nodiscard]] float GetW() const { return m_data.w; }

		[[nodiscard]] Quaternion operator*(const Quaternion& other) const { return Quaternion(m_data * other.m_data); }
		[[nodiscard]] Quaternion operator+(const Quaternion& other) const { return Quaternion(m_data + other.m_data); }
		[[nodiscard]] Quaternion operator-(const Quaternion& other) const { return Quaternion(m_data - other.m_data); }
		[[nodiscard]] Vector3	 operator*(const Vector3& other) const { return Vector3(m_data * other.Value()); }

		[[nodiscard]] static Quaternion AngleAxis(float angleInDegrees, float x, float y, float z)
		{
			return Quaternion(glm::angleAxis(glm::radians(angleInDegrees), glm::vec3(x, y, z)));
		}

		[[nodiscard]] static Quaternion AngleAxis(float angleInDegrees, const Vector3& axis)
		{
			return AngleAxis(angleInDegrees, axis.GetX(), axis.GetY(), axis.GetZ());
		}

		[[nodiscard]] Quaternion Normalized() const { return Quaternion(glm::normalize(m_data)); }

		[[nodiscard]] Quaternion Conjugated() const { return Quaternion(glm::conjugate(m_data)); }

		[[nodiscard]] float		Magnitude() const { return glm::length(m_data); }
		[[nodiscard]] bool		Equals(const Quaternion& other) const { return glm::all(glm::equal(m_data, other.m_data)); }
		[[nodiscard]] glm::quat Value() const { return m_data; }

		friend std::ostream& operator<<(std::ostream& os, const Quaternion& quat)
		{
			os << "Quaternion(" << quat.GetW() << ", " << quat.GetX() << ", " << quat.GetY() << ", " << quat.GetZ() << ")";
			return os;
		}

	private:
		glm::quat m_data;
	};
}
