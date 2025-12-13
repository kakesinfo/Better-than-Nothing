#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	int CInputManager::m_keys[KEYBOARD_KEY_COUNT] = {};

	float64 CInputManager::m_mouseX = 0;
	float64 CInputManager::m_mouseY = 0;
	float64 CInputManager::m_lastMouseX = 0;
	float64 CInputManager::m_lastMouseY = 0;
	float64 CInputManager::m_mouseScroll = 0;

	int CInputManager::m_buttons[MOUSE_BUTTON_COUNT] = {};

	void CInputManager::Initialize()
	{
		std::ranges::fill(m_keys, GLFW_RELEASE);
		std::ranges::fill(m_buttons, GLFW_RELEASE);
	}

	void CInputManager::UpdateKey(const int key, const int action)
	{
		m_keys[key] = action;
	}

	void CInputManager::PressKey(const int key)
	{
		m_keys[key] = GLFW_PRESS;
	}

	void CInputManager::ReleaseKey(const int key)
	{
		m_keys[key] = GLFW_RELEASE;
	}

	void CInputManager::UpdateMouseButton(const int button, const int action)
	{
		m_buttons[button] = action;
	}

	void CInputManager::PressMouseButton(const int button)
	{
		m_buttons[button] = GLFW_PRESS;
	}

	void CInputManager::ReleaseMouseButton(const int button)
	{
		m_buttons[button] = GLFW_RELEASE;
	}

	void CInputManager::UpdateMousePosition(const float64 x, const float64 y)
	{
		m_lastMouseX = m_mouseX;
		m_lastMouseY = m_mouseY;
		m_mouseX = x;
		m_mouseY = y;
	}

	void CInputManager::ResetMousePosition()
	{
		m_lastMouseX = m_mouseX;
		m_lastMouseY = m_mouseY;
	}

	glm::vec2 CInputManager::GetMousePosition()
	{
		return {m_mouseX, m_mouseY};
	}

	glm::vec2 CInputManager::GetLastMousePosition()
	{
		return {m_lastMouseX, m_lastMouseY};
	}

	glm::vec2 CInputManager::GetMouseDelta()
	{
		return GetMousePosition() - GetLastMousePosition();
	}

	void CInputManager::UpdateMouseScroll(const float64 y)
	{
		m_mouseScroll = y;
	}

	float64 CInputManager::GetMouseScroll()
	{
		return m_mouseScroll;
	}

	bool CInputManager::IsKeyPressed(const int key)
	{
		return m_keys[key] == GLFW_PRESS || m_keys[key] == GLFW_REPEAT;
	}

	bool CInputManager::IsMouseButtonPressed(const int button)
	{
		return m_buttons[button] == GLFW_PRESS || m_buttons[button] == GLFW_REPEAT;
	}
};
