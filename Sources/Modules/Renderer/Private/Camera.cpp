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

#include "Module_Renderer/Camera.hpp"

namespace BetterThanNothing
{
	Camera::Camera(const float fov, const float nearPlane, const float farPlane): m_fov(fov), m_nearPlane(nearPlane), m_farPlane(farPlane)
	{
		m_position = glm::vec3(2.0f, 2.0f, 2.0f);
		m_orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	}

	void Camera::SetPosition(const glm::vec3& position)
	{
		m_position = position;
	}

	void Camera::SetOrientation(const glm::quat& orientation)
	{
		m_orientation = orientation;
	}

	void Camera::Move(const glm::vec3& delta)
	{
		m_position += delta;
	}

	void Camera::Rotate(const float yaw, const float pitch)
	{
		const glm::quat qYaw = glm::angleAxis(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
		const glm::quat qPitch = glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));

		m_orientation = glm::normalize(qYaw * m_orientation * qPitch);
	}

	glm::mat4 Camera::GetViewMatrix() const
	{
		const glm::mat4 rotate = glm::mat4_cast(glm::conjugate(m_orientation));
		const glm::mat4 translate = glm::translate(glm::mat4(1.0f), -m_position);
		return rotate * translate;
	}

	glm::mat4 Camera::GetProjectionMatrix(const float aspectRatio) const
	{
		auto perspective = glm::perspective(glm::radians(m_fov), aspectRatio, m_nearPlane, m_farPlane);
		perspective[1][1] *= -1;
		return perspective;
	}

	glm::vec3 Camera::GetForward() const
	{
		return m_orientation * glm::vec3(0.0f, 0.0f, -1.0f);
	}

	glm::vec3 Camera::GetRight() const
	{
		return m_orientation * glm::vec3(1.0f, 0.0f, 0.0f);
	}

	glm::vec3 Camera::GetUp() const
	{
		return m_orientation * glm::vec3(0.0f, 1.0f, 0.0f);
	}

	glm::vec3 Camera::GetPosition() const
	{
		return m_position;
	}
}
