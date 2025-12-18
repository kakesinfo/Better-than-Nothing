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

#include "Module_Platform/CWindow.hpp"
#include "Module_Platform/Manager/CInputManager.hpp"

namespace BetterThanNothing
{
	CWindow::CWindow(std::string  title, const std::uint32_t width, const std::uint32_t height, const bool fullscreen, const bool resizable)
		: m_Title(std::move(title)), m_Width(width), m_Height(height), m_fullscreen(fullscreen), m_resizable(resizable)
	{
		if (glfwInit() == GLFW_FALSE)
			throw std::runtime_error("Failed to initialize GLFW");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, m_resizable ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

		GLFWmonitor* monitor = m_fullscreen
			? glfwGetPrimaryMonitor()
			: nullptr;

		m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), monitor, nullptr);

		if (m_Window == nullptr)
		{
			glfwTerminate();
			throw std::runtime_error("Failed to create GLFW window");
		}

		glfwSetWindowUserPointer(m_Window, this);
		glfwSetFramebufferSizeCallback(m_Window, ResizeCallback);
		glfwSetKeyCallback(m_Window, KeyCallback);
		glfwSetCursorPosCallback(m_Window, MouseCursorCallback);
		glfwSetScrollCallback(m_Window, MouseScrollCallback);
		glfwSetMouseButtonCallback(m_Window, MouseButtonCallback);
	}

	CWindow::~CWindow()
	{
		if (m_Window != nullptr)
		{
			glfwDestroyWindow(m_Window);
			glfwTerminate();
		}
	}

	GLFWwindow* CWindow::Handle() const
	{
		return m_Window;
	}

	void CWindow::PollEvents()
	{
		glfwPollEvents();
	}

	void CWindow::Show() const
	{
		glfwShowWindow(m_Window);
	}

	bool CWindow::ShouldClose() const
	{
		if (m_Window == nullptr)
			return true;
		return glfwWindowShouldClose(m_Window) == GLFW_TRUE;
	}

	std::pair<std::uint32_t, std::uint32_t> CWindow::GetFramebufferSize() const
	{
		int width, height;
		glfwGetFramebufferSize(m_Window, &width, &height);
		return {width, height};
	}

	bool CWindow::IsResized() const
	{
		return m_Resized;
	}

	void CWindow::SetResized(const bool resized)
	{
		m_Resized = resized;
	}

	void CWindow::SetWidth(const std::uint32_t width)
	{
		m_Width = width;
	}

	void CWindow::SetHeight(const std::uint32_t height)
	{
		m_Height = height;
	}

	void CWindow::ResizeCallback(GLFWwindow* window, const std::int32_t width, const std::int32_t height)
	{
		const auto windowPtr = static_cast<CWindow*>(glfwGetWindowUserPointer(window));
		windowPtr->SetWidth(width);
		windowPtr->SetHeight(height);
		windowPtr->SetResized(true);
	}

	void CWindow::KeyCallback(GLFWwindow* window, const std::int32_t key, const std::int32_t scancode, const std::int32_t action, const std::int32_t mods)
	{
		(void) scancode;
		(void) mods;

		if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}

		CInputManager::UpdateKey(key, action);
	}

	void CWindow::MouseCursorCallback(GLFWwindow* window, const double xpos, const double ypos)
	{
		(void) window;

		CInputManager::UpdateMousePosition(xpos, ypos);
	}

	void CWindow::MouseScrollCallback(GLFWwindow* window, const double xoffset, const double yoffset)
	{
		(void) window;
		(void) xoffset;

		CInputManager::UpdateMouseScroll(yoffset);
	}

	void CWindow::MouseButtonCallback(GLFWwindow* window, const std::int32_t button, const std::int32_t action, const std::int32_t mods)
	{
		(void) window;
		(void) mods;

		CInputManager::UpdateMouseButton(button, action);
	}
};
