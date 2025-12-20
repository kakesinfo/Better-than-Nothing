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

#include "Module_Platform/Window.hpp"
#include "Module_Platform/Manager/InputManager.hpp"

namespace BetterThanNothing
{
	Window::Window(std::string title, const std::uint32_t width, const std::uint32_t height, const bool fullscreen, const bool resizable)
		: m_title(std::move(title)), m_width(width), m_height(height), m_fullscreen(fullscreen), m_resizable(resizable)
	{
		if (glfwInit() == GLFW_FALSE)
			throw std::runtime_error("Failed to initialize GLFW");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, m_resizable ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

		GLFWmonitor* monitor = m_fullscreen ? glfwGetPrimaryMonitor() : nullptr;

		m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), monitor, nullptr);

		if (m_window == nullptr)
		{
			glfwTerminate();
			throw std::runtime_error("Failed to create GLFW window");
		}

		glfwSetWindowUserPointer(m_window, this);
		glfwSetFramebufferSizeCallback(m_window, ResizeCallback);
		glfwSetKeyCallback(m_window, KeyCallback);
		glfwSetCursorPosCallback(m_window, MouseCursorCallback);
		glfwSetScrollCallback(m_window, MouseScrollCallback);
		glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
	}

	Window::~Window()
	{
		if (m_window != nullptr)
		{
			glfwDestroyWindow(m_window);
			glfwTerminate();
		}
	}

	GLFWwindow* Window::Handle() const
	{
		return m_window;
	}

	void Window::PollEvents()
	{
		glfwPollEvents();
	}

	void Window::Show() const
	{
		glfwShowWindow(m_window);
	}

	bool Window::ShouldClose() const
	{
		if (m_window == nullptr)
			return true;
		return glfwWindowShouldClose(m_window) == GLFW_TRUE;
	}

	std::pair<std::uint32_t, std::uint32_t> Window::GetFramebufferSize() const
	{
		int width, height;
		glfwGetFramebufferSize(m_window, &width, &height);
		return {width, height};
	}

	bool Window::IsResized() const
	{
		return m_resized;
	}

	void Window::SetResized(const bool resized)
	{
		m_resized = resized;
	}

	void Window::SetWidth(const std::uint32_t width)
	{
		m_width = width;
	}

	void Window::SetHeight(const std::uint32_t height)
	{
		m_height = height;
	}

	void Window::ResizeCallback(GLFWwindow* window, const std::int32_t width, const std::int32_t height)
	{
		const auto windowPtr = static_cast<Window*>(glfwGetWindowUserPointer(window));
		windowPtr->SetWidth(width);
		windowPtr->SetHeight(height);
		windowPtr->SetResized(true);
	}

	void Window::KeyCallback(GLFWwindow* window, const std::int32_t key, const std::int32_t scancode, const std::int32_t action, const std::int32_t mods)
	{
		(void)scancode;
		(void)mods;

		if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
		{
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}

		InputManager::UpdateKey(key, action);
	}

	void Window::MouseCursorCallback(GLFWwindow* window, const double xpos, const double ypos)
	{
		(void)window;

		InputManager::UpdateMousePosition(xpos, ypos);
	}

	void Window::MouseScrollCallback(GLFWwindow* window, const double xoffset, const double yoffset)
	{
		(void)window;
		(void)xoffset;

		InputManager::UpdateMouseScroll(yoffset);
	}

	void Window::MouseButtonCallback(GLFWwindow* window, const std::int32_t button, const std::int32_t action, const std::int32_t mods)
	{
		(void)window;
		(void)mods;

		InputManager::UpdateMouseButton(button, action);
	}
};
