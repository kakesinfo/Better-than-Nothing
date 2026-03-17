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

#include "Scene/Camera.hpp"

#include "Module_Math/Matrix4.hpp"
#include "Module_Math/Quaternion.hpp"
#include "Module_Math/Vector3.hpp"

namespace BetterThanNothing
{
	Camera::Camera(const float fov, const float nearPlane, const float farPlane)
		: m_fov(fov), m_nearPlane(nearPlane), m_farPlane(farPlane)
	{
		m_position = Vector3(2.0f, 2.0f, 2.0f);
		m_orientation = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
	}

	Camera::~Camera() {}

	void Camera::SetPosition(const Vector3& position)
	{
		m_position = position;
	}

	void Camera::SetOrientation(const Quaternion& orientation)
	{
		m_orientation = orientation;
	}

	void Camera::Move(const Vector3& delta)
	{
		m_position += delta;
	}

	void Camera::Rotate(const float yaw, const float pitch)
	{
		const Quaternion qYaw = Quaternion::AngleAxis(yaw, Vector3::Up);
		const Quaternion qPitch = Quaternion::AngleAxis(pitch, Vector3::Right);

		m_orientation = (qYaw * m_orientation * qPitch).Normalized();
	}

	Matrix4 Camera::GetViewMatrix() const
	{
		const auto	  rotate = Matrix4(m_orientation.Conjugated());
		const Matrix4 translate = Matrix4::Identity.Translate(-m_position);
		return (rotate * translate);
	}

	Matrix4 Camera::GetProjectionMatrix(const float aspectRatio) const
	{
		return Matrix4::Perspective(m_fov, aspectRatio, m_nearPlane, m_farPlane);
	}

	Vector3 Camera::GetForward() const
	{
		return m_orientation * Vector3::Forward;
	}

	Vector3 Camera::GetRight() const
	{
		return m_orientation * Vector3::Right;
	}

	Vector3 Camera::GetUp() const
	{
		return m_orientation * Vector3::Up;
	}

	Vector3 Camera::GetPosition() const
	{
		return m_position;
	}
}
