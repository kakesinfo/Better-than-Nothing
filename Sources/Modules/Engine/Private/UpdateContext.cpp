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

#include "Module_Engine/UpdateContext.hpp"

#include <Module_Base/EKey.hpp>
#include <Module_Base/EMouseButton.hpp>
#include <Renderer/Camera.hpp>
#include <Module_Math/Vector2.hpp>
#include <Module_Math/Vector3.hpp>
#include <Module_Platform/Manager/InputManager.hpp>

namespace BetterThanNothing
{
	void UpdateContext::RotateCamera(const float yaw, const float pitch) const
	{
		m_camera->Rotate(yaw, pitch);
	}

	void UpdateContext::MoveCamera(const Vector3& delta) const
	{
		m_camera->Move(delta);
	}

	Vector3 UpdateContext::GetCameraForward() const
	{
		return m_camera->GetForward();
	}

	Vector3 UpdateContext::GetCameraRight() const
	{
		return m_camera->GetRight();
	}

	bool UpdateContext::IsKeyPressed(const EKey key) const
	{
		return InputManager::IsKeyPressed(key);
	}

	bool UpdateContext::IsMouseButtonPressed(const EMouseButton button) const
	{
		return InputManager::IsMouseButtonPressed(button);
	}

	Vector2 UpdateContext::GetMouseDelta() const
	{
		return InputManager::GetMouseDelta();
	}

	float UpdateContext::GetDeltaTime() const
	{
		return m_deltaTime;
	}
}
