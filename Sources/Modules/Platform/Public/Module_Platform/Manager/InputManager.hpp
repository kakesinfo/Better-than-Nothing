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

#pragma once
#include <Module_Base/EKey.hpp>
#include <Module_Base/EMouseButton.hpp>

namespace BetterThanNothing
{
	class Vector2;

	enum class EKey : std::int16_t;
	enum class EMouseButton : std::int16_t;

	struct InputManager
	{
	protected:
		static constexpr std::int32_t KEYBOARD_KEY_COUNT = 1024;
		static constexpr std::int32_t MOUSE_BUTTON_COUNT = 32;

		static EKeyAction m_keys[KEYBOARD_KEY_COUNT];

		static float m_mouseX;
		static float m_mouseY;
		static float m_lastMouseX;
		static float m_lastMouseY;
		static float m_mouseScroll;

		static EMouseButtonAction m_buttons[MOUSE_BUTTON_COUNT];

	public:
		explicit InputManager() = default;
		~InputManager() = default;

		static void Initialize();

		static void UpdateKey(int key, EKeyAction action);
		static void PressKey(int key);
		static void ReleaseKey(int key);

		static void UpdateMouseButton(int button, EMouseButtonAction action);
		static void PressMouseButton(int button);
		static void ReleaseMouseButton(int button);

		static void UpdateMousePosition(float x, float y);
		static void ResetMousePosition();

		static Vector2 GetMousePosition();
		static Vector2 GetLastMousePosition();
		static Vector2 GetMouseDelta();

		static void	 UpdateMouseScroll(float y);
		static float GetMouseScroll();

		static bool IsKeyPressed(EKey key);
		static bool IsKeyPressed(std::int16_t key);
		static bool IsMouseButtonPressed(EMouseButton button);
		static bool IsMouseButtonPressed(std::int16_t button);
	};
};
