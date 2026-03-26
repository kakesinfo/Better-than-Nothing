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

	float LegacyInputManager::m_mouseX = 0;
	float LegacyInputManager::m_mouseY = 0;
	float LegacyInputManager::m_lastMouseX = 0;
	float LegacyInputManager::m_lastMouseY = 0;
	float LegacyInputManager::m_mouseScroll = 0;

	EMouseButtonAction LegacyInputManager::m_buttons[MOUSE_BUTTON_COUNT] = {};

	void LegacyInputManager::Initialize()
	{
		std::ranges::fill(m_keys, EKeyAction::Released);
		std::ranges::fill(m_buttons, EMouseButtonAction::Released);
	}

	void LegacyInputManager::UpdateKey(const int key, const EKeyAction action)
	{
		m_keys[key] = action;
	}

	void LegacyInputManager::PressKey(const int key)
	{
		m_keys[key] = EKeyAction::Pressed;
	}

	void LegacyInputManager::ReleaseKey(const int key)
	{
		m_keys[key] = EKeyAction::Released;
	}

	void LegacyInputManager::UpdateMouseButton(const int button, const EMouseButtonAction action)
	{
		m_buttons[button] = action;
	}

	void LegacyInputManager::PressMouseButton(const int button)
	{
		m_buttons[button] = EMouseButtonAction::Pressed;
	}

	void LegacyInputManager::ReleaseMouseButton(const int button)
	{
		m_buttons[button] = EMouseButtonAction::Released;
	}

	void LegacyInputManager::UpdateMousePosition(const float x, const float y)
	{
		m_lastMouseX = m_mouseX;
		m_lastMouseY = m_mouseY;
		m_mouseX = x;
		m_mouseY = y;
	}

	void LegacyInputManager::ResetMousePosition()
	{
		m_lastMouseX = m_mouseX;
		m_lastMouseY = m_mouseY;
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
		return GetMousePosition() - GetLastMousePosition();
	}

	void LegacyInputManager::UpdateMouseScroll(const float y)
	{
		m_mouseScroll = y;
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
		return m_keys[key] == EKeyAction::Pressed; // or repeat ?
	}

	bool LegacyInputManager::IsMouseButtonPressed(EMouseButton button)
	{
		return IsMouseButtonPressed(static_cast<int16_t>(button));
	}
	bool LegacyInputManager::IsMouseButtonPressed(const int16_t button)
	{
		return m_buttons[button] == EMouseButtonAction::Pressed; // or repeat ?
	}
};
