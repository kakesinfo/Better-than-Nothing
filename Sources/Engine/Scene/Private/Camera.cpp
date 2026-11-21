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

#include "Math/Matrix4.hpp"
#include "Math/Quaternion.hpp"
#include "Math/Vector3.hpp"
#include <algorithm>

namespace BetterThanNothing
{
	Camera::Camera(const float fov, const float nearPlane, const float farPlane)
		: m_fov(fov), m_nearPlane(nearPlane), m_farPlane(farPlane), m_position(2.0f, 2.0f, 2.0f), m_orientation(Quaternion::Identity)
	{
	}

	void Camera::SetPosition(const Vector3& position)
	{
		m_position = position;
		m_viewDirty = true;
	}

	void Camera::SetOrientation(const Quaternion& orientation)
	{
		m_orientation = orientation.Normalized();
		const Vector3 euler = m_orientation.ToEulerDegrees();
		m_pitch = std::clamp(euler.GetX(), -89.0f, 89.0f);
		m_yaw = euler.GetY();
		m_viewDirty = true;
	}

	void Camera::SetFov(const float fov)
	{
		m_fov = std::clamp(fov, 1.0f, 179.0f);
	}

	void Camera::SetExposure(const float exposure)
	{
		m_exposure = std::max(0.01f, exposure);
	}

	void Camera::Move(const Vector3& delta)
	{
		if (delta == Vector3::Zero)
			return;
		m_position += delta;
		m_viewDirty = true;
	}

	void Camera::Rotate(const float yaw, const float pitch)
	{
		if (yaw == 0.0f && pitch == 0.0f)
			return;

		m_yaw += yaw;
		m_pitch = std::clamp(m_pitch + pitch, -89.0f, 89.0f);
		UpdateOrientationFromAngles();
	}

	void Camera::UpdateOrientationFromAngles()
	{
		const Quaternion qYaw = Quaternion::AngleAxis(m_yaw, Vector3::Up);
		const Quaternion qPitch = Quaternion::AngleAxis(m_pitch, Vector3::Right);
		m_orientation = (qYaw * qPitch).Normalized();
		m_viewDirty = true;
	}

	const Matrix4& Camera::GetViewMatrix() const
	{
		if (m_viewDirty)
		{
			const Matrix4 rotate(m_orientation.Conjugated());
			const Matrix4 translate = Matrix4::Identity.Translate(-m_position);
			m_viewMatrix = rotate * translate;
			m_viewDirty = false;
		}
		return m_viewMatrix;
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

	Quaternion Camera::GetOrientation() const
	{
		return m_orientation;
	}

	float Camera::GetFov() const
	{
		return m_fov;
	}

	float Camera::GetNearPlane() const
	{
		return m_nearPlane;
	}

	float Camera::GetFarPlane() const
	{
		return m_farPlane;
	}

	float Camera::GetExposure() const
	{
		return m_exposure;
	}
}
