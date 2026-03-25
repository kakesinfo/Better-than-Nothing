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

#include "Platform/Window.hpp"
#include "../Public/Platform/LegacyInputManager.hpp"

#include <Common/EKey.hpp>
#include <Common/Logger.hpp>

#include <format>

namespace BetterThanNothing
{
	Window::Window(std::string title, const uint32_t width, const uint32_t height, const bool fullscreen, const bool resizable)
		: m_title(std::move(title)), m_width(width), m_height(height), m_fullscreen(fullscreen), m_resizable(resizable)
	{
		uint32_t flags = SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN;
		flags |= m_fullscreen ? SDL_WINDOW_FULLSCREEN : 0;
		flags |= m_resizable ? SDL_WINDOW_RESIZABLE : 0;

		m_window = SDL_CreateWindow(m_title.c_str(), m_width, m_height, flags);

		if (!m_window)
			throw std::runtime_error(std::format("Cannot create SDL window: {}", SDL_GetError()));
	}

	Window::~Window()
	{
		SDL_DestroyWindow(m_window);
		SDL_Quit();
	}

	SDL_Window* Window::Handle() const
	{
		return m_window;
	}

	void Window::SetEventCallback(const std::function<bool(const SDL_Event&)>& callback)
	{
		m_eventCallback = callback;
	}

	void Window::PollEvents()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (m_eventCallback && m_eventCallback(event))
				continue;

			switch (event.type)
			{
				case SDL_EVENT_QUIT:
					m_shouldClose = true;
					break;

				case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
					ResizeCallback(this, event.window.data1, event.window.data2);
					break;

				case SDL_EVENT_KEY_DOWN:
					KeyCallback(event.key.scancode, EKeyAction::Pressed);
					break;

				case SDL_EVENT_KEY_UP:
					KeyCallback(event.key.scancode, EKeyAction::Released);
					break;

				case SDL_EVENT_MOUSE_MOTION:
					MouseCursorCallback(event.motion.x, event.motion.y);
					break;

				case SDL_EVENT_MOUSE_WHEEL:
					MouseScrollCallback(event.wheel.y);
					break;

				case SDL_EVENT_MOUSE_BUTTON_DOWN:
					MouseButtonCallback(event.button.button, EMouseButtonAction::Pressed);
					break;

				case SDL_EVENT_MOUSE_BUTTON_UP:
					MouseButtonCallback(event.button.button, EMouseButtonAction::Released);
					break;

				default:
					break;
			}
		}
	}

	void Window::Show() const
	{
		if (!SDL_ShowWindow(m_window))
			throw std::runtime_error(std::format("Cannot show SDL window: {}", SDL_GetError()));
	}

	bool Window::ShouldClose() const
	{
		if (m_window == nullptr)
			return true;
		return m_shouldClose;
	}

	std::pair<uint32_t, uint32_t> Window::GetFramebufferSize() const
	{
		int width, height;
		SDL_GetWindowSizeInPixels(m_window, &width, &height);
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

	void Window::SetWidth(const uint32_t width)
	{
		m_width = width;
	}

	void Window::SetHeight(const uint32_t height)
	{
		m_height = height;
	}

	void Window::ResizeCallback(Window* window, const int32_t width, const int32_t height)
	{
		window->SetWidth(width);
		window->SetHeight(height);
		window->SetResized(true);
	}

	void Window::KeyCallback(const int32_t key, const EKeyAction action)
	{
		LegacyInputManager::UpdateKey(key, action);
	}

	void Window::MouseCursorCallback(const double xpos, const double ypos)
	{
		LegacyInputManager::UpdateMousePosition(xpos, ypos);
	}

	void Window::MouseScrollCallback(const double yoffset)
	{
		LegacyInputManager::UpdateMouseScroll(yoffset);
	}

	void Window::MouseButtonCallback(const int32_t button, const EMouseButtonAction action)
	{

		LegacyInputManager::UpdateMouseButton(button, action);
	}
};
