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
	class Vector3;
	class Quaternion;

	class Matrix4
	{
	public:
		static const Matrix4 Identity;
		static const Matrix4 Zero;

		Matrix4();
		Matrix4(float value);
		Matrix4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20,
				float m21, float m22, float m23, float m30, float m31, float m32, float m33);

		explicit Matrix4(const float elements[16]);
		explicit Matrix4(const glm::mat4& glmMat);
		explicit Matrix4(const Quaternion& quat);

		[[nodiscard]] Matrix4 operator*(float value) const;
		[[nodiscard]] Matrix4 operator*(const Matrix4& other) const;
		[[nodiscard]] Matrix4 operator+(const Matrix4& other) const;
		[[nodiscard]] Matrix4 operator-(const Matrix4& other) const;

		[[nodiscard]] Matrix4 Translate(float x, float y, float z) const;
		[[nodiscard]] Matrix4 Translate(const Vector3& vec) const;

		[[nodiscard]] Matrix4 Rotate(float angleInDegrees, float x, float y, float z) const;
		[[nodiscard]] Matrix4 Rotate(float angleInDegrees, const Vector3& axis) const;

		[[nodiscard]] Matrix4 Scale(float x, float y, float z) const;
		[[nodiscard]] Matrix4 Scale(float uniformScale) const;
		[[nodiscard]] Matrix4 Scale(const Vector3& vec) const;

		[[nodiscard]] Matrix4	Transposed() const;
		[[nodiscard]] Matrix4	Inverted() const;
		[[nodiscard]] bool		Equals(const Matrix4& other) const;
		[[nodiscard]] glm::mat4 Value() const;

		static Matrix4 Perspective(float fovY, float aspect, float nearPlane, float farPlane);

		friend std::ostream& operator<<(std::ostream& os, const Matrix4& vec);

	private:
		glm::mat4 m_data;
	};
}