#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace BetterThanNothing
{
	enum class EKey : std::int16_t;
	enum class EMouseButton : std::int16_t;

	struct CInputManager {
	protected:
		static constexpr std::int32_t KEYBOARD_KEY_COUNT = 1024;
		static constexpr std::int32_t MOUSE_BUTTON_COUNT = 32;

		static int m_keys[KEYBOARD_KEY_COUNT];

		static float m_mouseX;
		static float m_mouseY;
		static float m_lastMouseX;
		static float m_lastMouseY;
		static float m_mouseScroll;

		static int m_buttons[MOUSE_BUTTON_COUNT];

	public:
		explicit CInputManager() = default;
		~CInputManager() = default;

		static void Initialize();

		static void UpdateKey(int key, int action);
		static void PressKey(int key);
		static void ReleaseKey(int key);

		static void UpdateMouseButton(int button, int action);
		static void PressMouseButton(int button);
		static void ReleaseMouseButton(int button);

		static void UpdateMousePosition(float x, float y);
		static void ResetMousePosition();

		static glm::vec2 GetMousePosition();
		static glm::vec2 GetLastMousePosition();
		static glm::vec2 GetMouseDelta();

		static void UpdateMouseScroll(float y);
		static float GetMouseScroll();

		static bool IsKeyPressed(EKey key);
		static bool IsKeyPressed(std::int16_t key);
		static bool IsMouseButtonPressed(EMouseButton button);
		static bool IsMouseButtonPressed(std::int16_t button);
	};
};
