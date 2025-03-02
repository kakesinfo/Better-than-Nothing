#include "../BetterThanNothing.hpp"

namespace BetterThanNothing
{
	int CInput::m_keys[KEYBOARD_KEY_COUNT] = {};

	float64 CInput::m_mouseX = 0;
	float64 CInput::m_mouseY = 0;
	float64 CInput::m_lastMouseX = 0;
	float64 CInput::m_lastMouseY = 0;
	float64 CInput::m_mouseScroll = 0;

	int CInput::m_buttons[MOUSE_BUTTON_COUNT] = {};

	CInput::CInput()
	{
		for (int & m_Key : m_keys) {
			m_Key = GLFW_RELEASE;
		}
	}

	void CInput::UpdateKey(const int key, const int action)
	{
		m_keys[key] = action;
	}

	void CInput::PressKey(const int key)
	{
		m_keys[key] = GLFW_PRESS;
	}

	void CInput::ReleaseKey(const int key)
	{
		m_keys[key] = GLFW_RELEASE;
	}

	void CInput::UpdateMouseButton(const int button, const int action)
	{
		m_buttons[button] = action;
	}

	void CInput::PressMouseButton(const int button)
	{
		m_buttons[button] = GLFW_PRESS;
	}

	void CInput::ReleaseMouseButton(const int button)
	{
		m_buttons[button] = GLFW_RELEASE;
	}

	void CInput::UpdateMousePosition(const float64 x, const float64 y)
	{
		m_lastMouseX = m_mouseX;
		m_lastMouseY = m_mouseY;
		m_mouseX = x;
		m_mouseY = y;
	}

	glm::vec2 CInput::GetMousePosition()
	{
		return {m_mouseX, m_mouseY};
	}

	glm::vec2 CInput::GetLastMousePosition()
	{
		return {m_lastMouseX, m_lastMouseY};
	}

	glm::vec2 CInput::GetMouseDelta()
	{
		return {m_mouseX - m_lastMouseX, m_lastMouseY - m_mouseY};
	}

	void CInput::UpdateMouseScroll(float64 y)
	{
		m_mouseScroll = y;
	}

	float64 CInput::GetMouseScroll()
	{
		return m_mouseScroll;
	}

	bool CInput::IsKeyPressed(const int key)
	{
		return m_keys[key] == GLFW_PRESS || m_keys[key] == GLFW_REPEAT;
	}

	bool CInput::IsMouseButtonPressed(const int button)
	{
		return m_buttons[button] == GLFW_PRESS || m_buttons[button] == GLFW_REPEAT;
	}
};
