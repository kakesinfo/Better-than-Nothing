#pragma once

namespace BetterThanNothing
{
	class CCamera
	{
	public:
		CCamera(float fov, float nearPlane, float farPlane);
		~CCamera() = default;

		void SetPosition(const glm::vec3& position);
		void SetOrientation(const glm::quat& orientation);

		void Move(const glm::vec3& delta);
		void Rotate(float yaw, float pitch);

		glm::mat4 GetViewMatrix() const;
		glm::mat4 GetProjectionMatrix(float aspectRatio) const;

		glm::vec3 GetForward() const;
		glm::vec3 GetRight() const;
		glm::vec3 GetUp() const;

	private:
		float		m_fov;
		float		m_nearPlane;
		float		m_farPlane;

		glm::vec3	m_position{};
		glm::quat	m_orientation{};
	};
}