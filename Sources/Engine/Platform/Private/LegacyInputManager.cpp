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

#include "Platform/LegacyInputManager.hpp"

#include <Common/EKey.hpp>
#include <Common/EMouseButton.hpp>
#include <Math/Vector2.hpp>
#include <algorithm>

namespace BetterThanNothing
{
	EKeyAction LegacyInputManager::m_keys[KEYBOARD_KEY_COUNT] = {};

	float LegacyInputManager::m_mouseX = 0.0f;
	float LegacyInputManager::m_mouseY = 0.0f;
	float LegacyInputManager::m_lastMouseX = 0.0f;
	float LegacyInputManager::m_lastMouseY = 0.0f;
	float LegacyInputManager::m_mouseDeltaX = 0.0f;
	float LegacyInputManager::m_mouseDeltaY = 0.0f;
	float LegacyInputManager::m_mouseScroll = 0.0f;
	bool  LegacyInputManager::m_hasMousePosition = false;

	EMouseButtonAction LegacyInputManager::m_buttons[MOUSE_BUTTON_COUNT] = {};

	void LegacyInputManager::Initialize()
	{
		std::ranges::fill(m_keys, EKeyAction::Released);
		std::ranges::fill(m_buttons, EMouseButtonAction::Released);
		m_mouseX = 0.0f;
		m_mouseY = 0.0f;
		m_lastMouseX = 0.0f;
		m_lastMouseY = 0.0f;
		m_mouseDeltaX = 0.0f;
		m_mouseDeltaY = 0.0f;
		m_mouseScroll = 0.0f;
		m_hasMousePosition = false;
	}

	void LegacyInputManager::EndFrame()
	{
		m_lastMouseX = m_mouseX;
		m_lastMouseY = m_mouseY;
		m_mouseDeltaX = 0.0f;
		m_mouseDeltaY = 0.0f;
		m_mouseScroll = 0.0f;
	}

	void LegacyInputManager::UpdateKey(const int key, const EKeyAction action)
	{
		if (key < 0 || key >= KEYBOARD_KEY_COUNT)
			return;
		m_keys[key] = action;
	}

	void LegacyInputManager::PressKey(const int key)
	{
		UpdateKey(key, EKeyAction::Pressed);
	}

	void LegacyInputManager::ReleaseKey(const int key)
	{
		UpdateKey(key, EKeyAction::Released);
	}

	void LegacyInputManager::UpdateMouseButton(const int button, const EMouseButtonAction action)
	{
		if (button < 0 || button >= MOUSE_BUTTON_COUNT)
			return;
		m_buttons[button] = action;
	}

	void LegacyInputManager::PressMouseButton(const int button)
	{
		UpdateMouseButton(button, EMouseButtonAction::Pressed);
	}

	void LegacyInputManager::ReleaseMouseButton(const int button)
	{
		UpdateMouseButton(button, EMouseButtonAction::Released);
	}

	void LegacyInputManager::UpdateMousePosition(const float x, const float y)
	{
		if (m_hasMousePosition)
		{
			m_mouseDeltaX += x - m_mouseX;
			m_mouseDeltaY += y - m_mouseY;
		}
		else
		{
			m_lastMouseX = x;
			m_lastMouseY = y;
			m_hasMousePosition = true;
		}

		m_mouseX = x;
		m_mouseY = y;
	}

	void LegacyInputManager::ResetMousePosition()
	{
		EndFrame();
	}

	Vector2 LegacyInputManager::GetMousePosition()
	{
		return {m_mouseX, m_mouseY};
	}

	Vector2 LegacyInputManager::GetLastMousePosition()
	{
		return {m_lastMouseX, m_lastMouseY};
	}

	Vector2 LegacyInputManager::GetMouseDelta()
	{
		return {m_mouseDeltaX, m_mouseDeltaY};
	}

	void LegacyInputManager::UpdateMouseScroll(const float y)
	{
		m_mouseScroll += y;
	}

	float LegacyInputManager::GetMouseScroll()
	{
		return m_mouseScroll;
	}

	bool LegacyInputManager::IsKeyPressed(EKey key)
	{
		return IsKeyPressed(static_cast<int16_t>(key));
	}

	bool LegacyInputManager::IsKeyPressed(const int16_t key)
	{
		if (key < 0 || key >= KEYBOARD_KEY_COUNT)
			return false;
		return m_keys[key] == EKeyAction::Pressed;
	}

	bool LegacyInputManager::IsMouseButtonPressed(EMouseButton button)
	{
		return IsMouseButtonPressed(static_cast<int16_t>(button));
	}
	bool LegacyInputManager::IsMouseButtonPressed(const int16_t button)
	{
		if (button < 0 || button >= MOUSE_BUTTON_COUNT)
			return false;
		return m_buttons[button] == EMouseButtonAction::Pressed;
	}
};
