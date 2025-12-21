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

#include "Module_Platform/Manager/InputManager.hpp"

#include <Module_Base/EKey.hpp>
#include <Module_Base/EMouseButton.hpp>
#include <Module_Math/Vector2.hpp>

namespace BetterThanNothing
{
	int InputManager::m_keys[KEYBOARD_KEY_COUNT] = {};

	float InputManager::m_mouseX = 0;
	float InputManager::m_mouseY = 0;
	float InputManager::m_lastMouseX = 0;
	float InputManager::m_lastMouseY = 0;
	float InputManager::m_mouseScroll = 0;

	int InputManager::m_buttons[MOUSE_BUTTON_COUNT] = {};

	void InputManager::Initialize()
	{
		std::ranges::fill(m_keys, GLFW_RELEASE);
		std::ranges::fill(m_buttons, GLFW_RELEASE);
	}

	void InputManager::UpdateKey(const int key, const int action)
	{
		m_keys[key] = action;
	}

	void InputManager::PressKey(const int key)
	{
		m_keys[key] = GLFW_PRESS;
	}

	void InputManager::ReleaseKey(const int key)
	{
		m_keys[key] = GLFW_RELEASE;
	}

	void InputManager::UpdateMouseButton(const int button, const int action)
	{
		m_buttons[button] = action;
	}

	void InputManager::PressMouseButton(const int button)
	{
		m_buttons[button] = GLFW_PRESS;
	}

	void InputManager::ReleaseMouseButton(const int button)
	{
		m_buttons[button] = GLFW_RELEASE;
	}

	void InputManager::UpdateMousePosition(const float x, const float y)
	{
		m_lastMouseX = m_mouseX;
		m_lastMouseY = m_mouseY;
		m_mouseX = x;
		m_mouseY = y;
	}

	void InputManager::ResetMousePosition()
	{
		m_lastMouseX = m_mouseX;
		m_lastMouseY = m_mouseY;
	}

	Vector2 InputManager::GetMousePosition()
	{
		return {m_mouseX, m_mouseY};
	}

	Vector2 InputManager::GetLastMousePosition()
	{
		return {m_lastMouseX, m_lastMouseY};
	}

	Vector2 InputManager::GetMouseDelta()
	{
		return GetMousePosition() - GetLastMousePosition();
	}

	void InputManager::UpdateMouseScroll(const float y)
	{
		m_mouseScroll = y;
	}

	float InputManager::GetMouseScroll()
	{
		return m_mouseScroll;
	}

	bool InputManager::IsKeyPressed(const EKey key)
	{
		return InputManager::IsKeyPressed(static_cast<std::int16_t>(key));
	}

	bool InputManager::IsKeyPressed(const std::int16_t key)
	{
		return m_keys[key] == GLFW_PRESS || m_keys[key] == GLFW_REPEAT;
	}

	bool InputManager::IsMouseButtonPressed(const EMouseButton button)
	{
		return InputManager::IsMouseButtonPressed(static_cast<int>(button));
	}

	bool InputManager::IsMouseButtonPressed(const std::int16_t button)
	{
		return m_buttons[button] == GLFW_PRESS || m_buttons[button] == GLFW_REPEAT;
	}
};
