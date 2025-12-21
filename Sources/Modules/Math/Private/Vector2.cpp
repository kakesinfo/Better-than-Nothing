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

#include "Module_Math/Vector2.hpp"

namespace BetterThanNothing
{
	const Vector2 Vector2::Down = Vector2(0.0f, -1.0f);
	const Vector2 Vector2::Left = Vector2(-1.0f, 0.0f);
	const Vector2 Vector2::One = Vector2(1.0f, 1.0f);
	const Vector2 Vector2::Right = Vector2(1.0f, 0.0f);
	const Vector2 Vector2::Up = Vector2(0.0f, 1.0f);
	const Vector2 Vector2::Zero = Vector2(0.0f, 0.0f);

	Vector2::Vector2() : m_data(0.0f, 0.0f) {}
	Vector2::Vector2(const float x, const float y) : m_data(x, y) {}
	Vector2::Vector2(const glm::vec2& other) : m_data(other) {}
	Vector2::Vector2(const float value) : m_data(value) {}

	Vector2::Vector2(const Vector2& other) : m_data(other.m_data) {}

	float Vector2::GetX() const
	{
		return m_data.x;
	}
	float Vector2::GetY() const
	{
		return m_data.y;
	}

	void Vector2::SetX(const float x)
	{
		m_data.x = x;
	}
	void Vector2::SetY(const float y)
	{
		m_data.y = y;
	}

	Vector2 Vector2::operator+(const Vector2& other) const
	{
		return Vector2(m_data + other.m_data);
	}

	Vector2 Vector2::operator-(const Vector2& other) const
	{
		return Vector2(m_data - other.m_data);
	}

	Vector2 Vector2::operator*(const float scalar) const
	{
		return Vector2(m_data * scalar);
	}

	Vector2 Vector2::operator/(const float scalar) const
	{
		return Vector2(m_data / scalar);
	}

	Vector2 Vector2::operator-() const
	{
		return Vector2(-m_data);
	}

	Vector2 Vector2::operator+=(const Vector2& other)
	{
		m_data += other.m_data;
		return *this;
	}

	Vector2 Vector2::operator-=(const Vector2& other)
	{
		m_data -= other.m_data;
		return *this;
	}

	Vector2 Vector2::operator*=(const float scalar)
	{
		m_data *= scalar;
		return *this;
	}

	Vector2 Vector2::operator/=(const float scalar)
	{
		m_data /= scalar;
		return *this;
	}

	Vector2 Vector2::operator=(const Vector2& other)
	{
		m_data = other.m_data;
		return *this;
	}

	bool Vector2::Equals(const Vector2& other) const
	{
		return m_data == other.m_data;
	}

	float Vector2::Magnitude() const
	{
		return glm::length(m_data);
	}

	Vector2 Vector2::Normalized() const
	{
		return Vector2(glm::normalize(m_data));
	}

	glm::vec2 Vector2::Value() const
	{
		return m_data;
	}

	float Vector2::Dot(const Vector2& a, const Vector2& b)
	{
		return glm::dot(a.m_data, b.m_data);
	}

	Vector2 Vector2::Lerp(const Vector2& a, const Vector2& b, const float t)
	{
		return Vector2(glm::mix(a.m_data, b.m_data, t));
	}

	Vector2 Vector2::Min(const Vector2& a, const Vector2& b)
	{
		return Vector2(glm::min(a.m_data, b.m_data));
	}

	Vector2 Vector2::Max(const Vector2& a, const Vector2& b)
	{
		return Vector2(glm::max(a.m_data, b.m_data));
	}

	float Vector2::Distance(const Vector2& a, const Vector2& b)
	{
		return glm::distance(a.m_data, b.m_data);
	}

	std::ostream& operator<<(std::ostream& os, const Vector2& vec)
	{
		os << "Vector2(" << vec.GetX() << ", " << vec.GetY() << ")";
		return os;
	}
}