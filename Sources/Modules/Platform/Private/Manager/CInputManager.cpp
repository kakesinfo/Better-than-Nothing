#include "module_platform.hpp"

#include <Module_Base/EKey.hpp>
#include <Module_Base/EMouseButton.hpp>

namespace BetterThanNothing
{
	int CInputManager::m_keys[KEYBOARD_KEY_COUNT] = {};

	float CInputManager::m_mouseX = 0;
	float CInputManager::m_mouseY = 0;
	float CInputManager::m_lastMouseX = 0;
	float CInputManager::m_lastMouseY = 0;
	float CInputManager::m_mouseScroll = 0;

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

	void CInputManager::UpdateMousePosition(const float x, const float y)
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

	void CInputManager::UpdateMouseScroll(const float y)
	{
		m_mouseScroll = y;
	}

	float CInputManager::GetMouseScroll()
	{
		return m_mouseScroll;
	}

	bool CInputManager::IsKeyPressed(const EKey key)
	{
		return CInputManager::IsKeyPressed(static_cast<std::int16_t>(key));
	}

	bool CInputManager::IsKeyPressed(const std::int16_t key)
	{
		return m_keys[key] == GLFW_PRESS || m_keys[key] == GLFW_REPEAT;
	}

	bool CInputManager::IsMouseButtonPressed(const EMouseButton button)
	{
		return CInputManager::IsMouseButtonPressed(static_cast<int>(button));
	}

	bool CInputManager::IsMouseButtonPressed(const std::int16_t button)
	{
		return m_buttons[button] == GLFW_PRESS || m_buttons[button] == GLFW_REPEAT;
	}
};
