#include "Module_Engine/CUpdateContext.hpp"
#include "Module_Base/EKey.hpp"
#include "Module_Base/EMouseButton.hpp"
#include "Module_Platform/Manager/CInputManager.hpp"
#include "Module_Renderer/CCamera.hpp"

namespace BetterThanNothing
{
	void CUpdateContext::RotateCamera(const float yaw, const float pitch) const
	{
		m_camera->Rotate(yaw, pitch);
	}

	void CUpdateContext::MoveCamera(const glm::vec3& delta) const
	{
		m_camera->Move(delta);
	}

	glm::vec3 CUpdateContext::GetCameraForward() const
	{
		return m_camera->GetForward();
	}

	glm::vec3 CUpdateContext::GetCameraRight() const
	{
		return m_camera->GetRight();
	}

	bool CUpdateContext::IsKeyPressed(const EKey key) const
	{
		return CInputManager::IsKeyPressed(key);
	}

	bool CUpdateContext::IsMouseButtonPressed(const EMouseButton button) const
	{
		return CInputManager::IsMouseButtonPressed(button);
	}

	glm::vec2 CUpdateContext::GetMouseDelta() const
	{
		return CInputManager::GetMouseDelta();
	}

	float CUpdateContext::GetDeltaTime() const
	{
		return m_deltaTime;
	}
}