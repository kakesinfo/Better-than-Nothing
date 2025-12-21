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

#include "Module_Math/Matrix4.hpp"
#include "Module_Math/Quaternion.hpp"
#include "Module_Math/Vector3.hpp"

namespace BetterThanNothing
{
	const Matrix4 Matrix4::Identity = Matrix4(glm::mat4(1.0f));
	const Matrix4 Matrix4::Zero = Matrix4(glm::mat4(0.0f));

	Matrix4::Matrix4() : m_data(0.0f) {}
	Matrix4::Matrix4(const float value) : m_data(value) {}
	Matrix4::Matrix4(const float m00, const float m01, const float m02, const float m03, const float m10,
					 const float m11, const float m12, const float m13, const float m20, const float m21,
					 const float m22, const float m23, const float m30, const float m31, const float m32,
					 const float m33)
		: m_data(m00, m10, m20, m30, m01, m11, m21, m31, m02, m12, m22, m32, m03, m13, m23, m33)
	{}

	Matrix4::Matrix4(const float elements[16])
		: m_data(elements[0], elements[4], elements[8], elements[12], elements[1], elements[5], elements[9],
				  elements[13], elements[2], elements[6], elements[10], elements[14], elements[3], elements[7],
				  elements[11], elements[15])
	{}

	Matrix4::Matrix4(const glm::mat4& glmMat) : m_data(glmMat) {}

	Matrix4::Matrix4(const Quaternion& quat) : m_data(glm::mat4_cast(quat.Value())) {}

	Matrix4 Matrix4::operator*(float value) const
	{
		return Matrix4(m_data * value);
	}

	Matrix4 Matrix4::operator*(const Matrix4& other) const
	{
		return Matrix4(m_data * other.m_data);
	}

	Matrix4 Matrix4::operator+(const Matrix4& other) const
	{
		return Matrix4(m_data + other.m_data);
	}

	Matrix4 Matrix4::operator-(const Matrix4& other) const
	{
		return Matrix4(m_data - other.m_data);
	}

	Matrix4 Matrix4::Translate(const float x, const float y, const float z) const
	{
		return Matrix4(glm::translate(m_data, glm::vec3(x, y, z)));
	}

	Matrix4 Matrix4::Translate(const Vector3& vec) const
	{
		return Translate(vec.GetX(), vec.GetY(), vec.GetZ());
	}

	Matrix4 Matrix4::Rotate(const float angleInDegrees, const float x, const float y, const float z) const
	{
		return Matrix4(glm::rotate(m_data, glm::radians(angleInDegrees), glm::vec3(x, y, z)));
	}

	Matrix4 Matrix4::Rotate(const float angleInDegrees, const Vector3& axis) const
	{
		return Rotate(angleInDegrees, axis.GetX(), axis.GetY(), axis.GetZ());
	}

	Matrix4 Matrix4::Scale(const float x, const float y, const float z) const
	{
		return Matrix4(glm::scale(m_data, glm::vec3(x, y, z)));
	}

	Matrix4 Matrix4::Scale(const float uniformScale) const
	{
		return Scale(uniformScale, uniformScale, uniformScale);
	}

	Matrix4 Matrix4::Scale(const Vector3& vec) const
	{
		return Scale(vec.GetX(), vec.GetY(), vec.GetZ());
	}

	Matrix4 Matrix4::Transposed() const
	{
		return Matrix4(glm::transpose(m_data));
	}

	Matrix4 Matrix4::Inverted() const
	{
		return Matrix4(glm::inverse(m_data));
	}

	bool Matrix4::Equals(const Matrix4& other) const
	{
		return m_data == other.m_data;
	}

	glm::mat4 Matrix4::Value() const
	{
		return m_data;
	}

	Matrix4 Matrix4::Perspective(float fovY, float aspect, float nearPlane, float farPlane)
	{
		auto perspective = glm::perspective(glm::radians(fovY), aspect, nearPlane, farPlane);
		perspective[1][1] *= -1; // Invert Y for Vulkan
		return Matrix4(perspective);
	}

	std::ostream& operator<<(std::ostream& os, const Matrix4& mat)
	{
		for (int row = 0; row < 4; ++row)
		{
			os << "| ";
			for (int col = 0; col < 4; ++col)
				os << mat.m_data[col][row] << " ";
			os << "|\n";
		}
		return os;
	}
}