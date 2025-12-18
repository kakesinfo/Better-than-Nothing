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