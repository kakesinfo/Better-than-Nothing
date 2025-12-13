#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CCamera::CCamera(const float fov, const float nearPlane, const float farPlane): m_fov(fov), m_nearPlane(nearPlane), m_farPlane(farPlane)
	{
		m_position = glm::vec3(2.0f, 2.0f, 2.0f);
		m_orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	}

	void CCamera::SetPosition(const glm::vec3& position)
	{
		m_position = position;
	}

	void CCamera::SetOrientation(const glm::quat& orientation)
	{
		m_orientation = orientation;
	}

	void CCamera::Move(const glm::vec3& delta)
	{
		m_position += delta;
	}

	void CCamera::Rotate(const float yaw, const float pitch)
	{
		const glm::quat qYaw = glm::angleAxis(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
		const glm::quat qPitch = glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));

		m_orientation = glm::normalize(qYaw * m_orientation * qPitch);
	}

	glm::mat4 CCamera::GetViewMatrix() const
	{
		const glm::mat4 rotate = glm::mat4_cast(glm::conjugate(m_orientation));
		const glm::mat4 translate = glm::translate(glm::mat4(1.0f), -m_position);
		return rotate * translate;
	}

	glm::mat4 CCamera::GetProjectionMatrix(const float aspectRatio) const
	{
		auto perspective = glm::perspective(glm::radians(m_fov), aspectRatio, m_nearPlane, m_farPlane);
		perspective[1][1] *= -1;
		return perspective;
	}

	glm::vec3 CCamera::GetForward() const
	{
		return m_orientation * glm::vec3(0.0f, 0.0f, -1.0f);
	}

	glm::vec3 CCamera::GetRight() const
	{
		return m_orientation * glm::vec3(1.0f, 0.0f, 0.0f);
	}

	glm::vec3 CCamera::GetUp() const
	{
		return m_orientation * glm::vec3(0.0f, 1.0f, 0.0f);
	}

	glm::vec3 CCamera::GetPosition() const
	{
		return m_position;
	}
}
