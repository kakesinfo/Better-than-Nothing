#pragma once

namespace BetterThanNothing
{
	class CCamera;
	enum class EKey : std::int16_t;
	enum class EMouseButton : std::int16_t;

	class CUpdateContext
	{
	public:
		explicit CUpdateContext(CCamera* camera, const float deltaTime): m_camera(camera), m_deltaTime(deltaTime) {}
		~CUpdateContext() = default;

		void RotateCamera(float yaw, float pitch) const;
		void MoveCamera(const glm::vec3& delta) const;

		[[nodiscard]] glm::vec3 GetCameraForward() const;
		[[nodiscard]] glm::vec3 GetCameraRight() const;

		[[nodiscard]] bool IsKeyPressed(EKey key) const;
		[[nodiscard]] bool IsMouseButtonPressed(EMouseButton button) const;
		[[nodiscard]] glm::vec2 GetMouseDelta() const;

		[[nodiscard]] float GetDeltaTime() const;

	private:
		CCamera*	m_camera;
		float		m_deltaTime;
	};
}
