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

#include "Module_Math/Quaternion.hpp"
#include "Module_Math/Vector3.hpp"

namespace BetterThanNothing
{
	class Matrix4;

	class Camera
	{
	public:
		Camera(float fov, float nearPlane, float farPlane);
		~Camera();

		void SetPosition(const Vector3& position);
		void SetOrientation(const Quaternion& orientation);

		void Move(const Vector3& delta);
		void Rotate(float yaw, float pitch);

		Matrix4 GetViewMatrix() const;
		Matrix4 GetProjectionMatrix(float aspectRatio) const;

		Vector3 GetForward() const;
		Vector3 GetRight() const;
		Vector3 GetUp() const;

		Vector3 GetPosition() const;

	private:
		float m_fov;
		float m_nearPlane;
		float m_farPlane;

		Vector3 m_position;
		Quaternion m_orientation;
	};
}