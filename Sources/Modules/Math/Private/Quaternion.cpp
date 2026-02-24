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

#include "Module_Math/Quaternion.hpp"
#include "Module_Math/Vector3.hpp"

namespace BetterThanNothing
{
	const Quaternion Quaternion::Identity = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);

	Quaternion::Quaternion() : m_data(0.0f, 0.0f, 0.0f, 0.0f) {}
	Quaternion::Quaternion(const float w, const float x, const float y, const float z) : m_data(w, x, y, z) {}
	Quaternion::Quaternion(const glm::quat& quat) : m_data(quat) {}

	float Quaternion::GetX() const
	{
		return m_data.x;
	}

	float Quaternion::GetY() const
	{
		return m_data.y;
	}

	float Quaternion::GetZ() const
	{
		return m_data.z;
	}

	float Quaternion::GetW() const
	{
		return m_data.w;
	}

	Quaternion Quaternion::operator*(const Quaternion& other) const
	{
		return Quaternion(m_data * other.m_data);
	}

	Quaternion Quaternion::operator+(const Quaternion& other) const
	{
		return Quaternion(m_data + other.m_data);
	}

	Quaternion Quaternion::operator-(const Quaternion& other) const
	{
		return Quaternion(m_data - other.m_data);
	}

	Vector3 Quaternion::operator*(const Vector3& other) const
	{
		return Vector3(m_data * other.Value());
	}

	Quaternion Quaternion::AngleAxis(const float angleInDegrees, const float x, const float y, const float z)
	{
		return Quaternion(glm::angleAxis(glm::radians(angleInDegrees), glm::vec3(x, y, z)));
	}

	Quaternion Quaternion::AngleAxis(const float angleInDegrees, const Vector3& axis)
	{
		return AngleAxis(angleInDegrees, axis.GetX(), axis.GetY(), axis.GetZ());
	}

	Quaternion Quaternion::Normalized() const
	{
		return Quaternion(glm::normalize(m_data));
	}

	Quaternion Quaternion::Conjugated() const
	{
		return Quaternion(glm::conjugate(m_data));
	}

	float Quaternion::Magnitude() const
	{
		return glm::length(m_data);
	}

	bool Quaternion::Equals(const Quaternion& other) const
	{
		return glm::all(glm::equal(m_data, other.m_data));
	}

	glm::quat Quaternion::Value() const
	{
		return m_data;
	}

	std::ostream& operator<<(std::ostream& os, const Quaternion& quat)
	{
		os << "Quaternion(" << quat.GetW() << ", " << quat.GetX() << ", " << quat.GetY() << ", " << quat.GetZ() << ")";
		return os;
	}
}
