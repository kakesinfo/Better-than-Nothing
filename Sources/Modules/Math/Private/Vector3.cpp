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

#include "Module_Math/Vector3.hpp"

namespace BetterThanNothing
{
	const Vector3 Vector3::Down = Vector3(0.0f, -1.0f, 0.0f);
	const Vector3 Vector3::Left = Vector3(-1.0f, 0.0f, 0.0f);
	const Vector3 Vector3::One = Vector3(1.0f, 1.0f, 1.0f);
	const Vector3 Vector3::Right = Vector3(1.0f, 0.0f, 0.0f);
	const Vector3 Vector3::Up = Vector3(0.0f, 1.0f, 0.0f);
	const Vector3 Vector3::Forward = Vector3(0.0f, 0.0f, -1.0f);
	const Vector3 Vector3::Zero = Vector3(0.0f, 0.0f, 0.0f);

	Vector3::Vector3() : m_data(0.0f, 0.0f, 0.0f) {}
	Vector3::Vector3(const float x, const float y, const float z) : m_data(x, y, z) {}
	Vector3::Vector3(const glm::vec3& other) : m_data(other) {}
	Vector3::Vector3(const float value) : m_data(value) {}

	Vector3::Vector3(const Vector3& other) : m_data(other.m_data) {}

	float Vector3::GetX() const
	{
		return m_data.x;
	}

	float Vector3::GetY() const
	{
		return m_data.y;
	}

	float Vector3::GetZ() const
	{
		return m_data.z;
	}

	void Vector3::SetX(const float x)
	{
		m_data.x = x;
	}

	void Vector3::SetY(const float y)
	{
		m_data.y = y;
	}

	void Vector3::SetZ(const float z)
	{
		m_data.z = z;
	}

	Vector3 Vector3::operator+(const Vector3& other) const
	{
		return Vector3(m_data + other.m_data);
	}

	Vector3 Vector3::operator-(const Vector3& other) const
	{
		return Vector3(m_data - other.m_data);
	}

	Vector3 Vector3::operator*(const float scalar) const
	{
		return Vector3(m_data * scalar);
	}

	Vector3 Vector3::operator/(const float scalar) const
	{
		return Vector3(m_data / scalar);
	}

	Vector3 Vector3::operator-() const
	{
		return Vector3(-m_data);
	}

	Vector3 Vector3::operator+=(const Vector3& other)
	{
		m_data += other.m_data;
		return *this;
	}

	Vector3 Vector3::operator-=(const Vector3& other)
	{
		m_data -= other.m_data;
		return *this;
	}

	Vector3 Vector3::operator*=(const float scalar)
	{
		m_data *= scalar;
		return *this;
	}

	Vector3 Vector3::operator/=(const float scalar)
	{
		m_data /= scalar;
		return *this;
	}

	Vector3 Vector3::operator=(const Vector3& other)
	{
		m_data = other.m_data;
		return *this;
	}

	bool Vector3::Equals(const Vector3& other) const
	{
		return m_data == other.m_data;
	}

	float Vector3::Magnitude() const
	{
		return glm::length(m_data);
	}

	Vector3 Vector3::Normalized() const
	{
		return Vector3(glm::normalize(m_data));
	}

	glm::vec3 Vector3::Value() const
	{
		return m_data;
	}

	float Vector3::Dot(const Vector3& a, const Vector3& b)
	{
		return glm::dot(a.m_data, b.m_data);
	}

	Vector3 Vector3::Lerp(const Vector3& a, const Vector3& b, const float t)
	{
		return Vector3(glm::mix(a.m_data, b.m_data, t));
	}

	Vector3 Vector3::Min(const Vector3& a, const Vector3& b)
	{
		return Vector3(glm::min(a.m_data, b.m_data));
	}

	Vector3 Vector3::Max(const Vector3& a, const Vector3& b)
	{
		return Vector3(glm::max(a.m_data, b.m_data));
	}

	float Vector3::Distance(const Vector3& a, const Vector3& b)
	{
		return glm::distance(a.m_data, b.m_data);
	}

	std::ostream& operator<<(std::ostream& os, const Vector3& vec)
	{
		os << "Vector3(" << vec.GetX() << ", " << vec.GetY() << ")";
		return os;
	}
}
