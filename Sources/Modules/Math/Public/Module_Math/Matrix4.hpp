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

#include "Quaternion.hpp"
#include "Vector3.hpp"

#include <ostream>

namespace BetterThanNothing
{
	class Matrix4
	{
	public:
		static const Matrix4 Identity;
		static const Matrix4 Zero;

		Matrix4() : m_data(0.0f) {}

		// clang-format off
		Matrix4(
			float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33
		) : m_data(
			m00, m10, m20, m30,
			m01, m11, m21, m31,
			m02, m12, m22, m32,
			m03, m13, m23, m33
		) {}
		// clang-format on

		// clang-format off
		explicit Matrix4(
			const float elements[16]
		) : m_data(
			elements[0], elements[4], elements[8], elements[12],
			elements[1], elements[5], elements[9], elements[13],
			elements[2], elements[6], elements[10], elements[14],
			elements[3], elements[7], elements[11], elements[15]
		) {}
		// clang-format on

		explicit Matrix4(float value) : m_data(value) {}
		explicit Matrix4(const glm::mat4& glmMat) : m_data(glmMat) {}
		explicit Matrix4(const Quaternion& quat) : m_data(glm::mat4_cast(quat.Value())) {}

		[[nodiscard]] Matrix4 operator*(float value) const { return Matrix4(m_data * value); }
		[[nodiscard]] Matrix4 operator*(const Matrix4& other) const { return Matrix4(m_data * other.m_data); }
		[[nodiscard]] Matrix4 operator+(const Matrix4& other) const { return Matrix4(m_data + other.m_data); }
		[[nodiscard]] Matrix4 operator-(const Matrix4& other) const { return Matrix4(m_data - other.m_data); }

		[[nodiscard]] Matrix4 Rotate(float angleInDegrees, float x, float y, float z) const
		{
			return Matrix4(glm::rotate(m_data, glm::radians(angleInDegrees), glm::vec3(x, y, z)));
		}

		[[nodiscard]] Matrix4 Rotate(const float angleInDegrees, const Vector3& axis) const
		{
			return Rotate(angleInDegrees, axis.GetX(), axis.GetY(), axis.GetZ());
		}

		[[nodiscard]] Matrix4	Translate(float x, float y, float z) const { return Matrix4(glm::translate(m_data, glm::vec3(x, y, z))); }
		[[nodiscard]] Matrix4	Translate(const Vector3& vec) const { return Translate(vec.GetX(), vec.GetY(), vec.GetZ()); }
		[[nodiscard]] Matrix4	Scale(float x, float y, float z) const { return Matrix4(glm::scale(m_data, glm::vec3(x, y, z))); }
		[[nodiscard]] Matrix4	Scale(const float uniformScale) const { return Scale(uniformScale, uniformScale, uniformScale); }
		[[nodiscard]] Matrix4	Scale(const Vector3& vec) const { return Scale(vec.GetX(), vec.GetY(), vec.GetZ()); }
		[[nodiscard]] Matrix4	Transposed() const { return Matrix4(glm::transpose(m_data)); }
		[[nodiscard]] Matrix4	Inverted() const { return Matrix4(glm::inverse(m_data)); }
		[[nodiscard]] bool		Equals(const Matrix4& other) const { return m_data == other.m_data; }
		[[nodiscard]] glm::mat4 Value() const { return m_data; }

		static Matrix4 Perspective(float fovY, float aspect, float nearPlane, float farPlane)
		{
			auto perspective = glm::perspective(glm::radians(fovY), aspect, nearPlane, farPlane);
			perspective[1][1] *= -1; // Invert Y for Vulkan
			return Matrix4(perspective);
		}

		friend std::ostream& operator<<(std::ostream& os, const Matrix4& mat)
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

	private:
		glm::mat4 m_data;
	};
}
