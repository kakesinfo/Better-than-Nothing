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

#include "Platform/Platform.hpp"
#include "Platform/LegacyInputManager.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <format>
#include <functional>
#include <utility>

namespace BetterThanNothing
{
	struct PlatformInternals
	{
		SDL_Window* window;
	};

	Platform::Platform(const PlatformCreateInfo& createInfo) : m_createInfo(createInfo)
	{
		m_self = std::make_unique<PlatformInternals>();
		m_running = true;
		m_windowWidth = createInfo.windowWidth;
		m_windowHeight = createInfo.windowHeight;
		m_windowResized = false;

		if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
			throw std::runtime_error("Cannot init SDL: " + std::string(SDL_GetError()));

		SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN;
		flags |= createInfo.windowFullscreen ? SDL_WINDOW_FULLSCREEN : 0;
		flags |= createInfo.windowResizable ? SDL_WINDOW_RESIZABLE : 0;

		m_self->window = SDL_CreateWindow(createInfo.windowTitle.c_str(), createInfo.windowWidth, createInfo.windowHeight, flags);

		if (!m_self->window)
			throw std::runtime_error("Cannot create SDL window: " + std::string(SDL_GetError()));
	}

	Platform::~Platform()
	{
		SDL_DestroyWindow(m_self->window);
		SDL_Quit();
	}

	std::unique_ptr<Platform> Platform::Create(std::string windowTitle, const int32_t windowWidth, const int32_t windowHeight)
	{
		PlatformCreateInfo createInfo;
		createInfo.windowTitle = std::move(windowTitle);
		createInfo.windowWidth = windowWidth;
		createInfo.windowHeight = windowHeight;
		createInfo.windowFullscreen = false;
		createInfo.windowResizable = false;

		return std::make_unique<Platform>(createInfo);
	}

	void Platform::PollEvents()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			// TODO: Add event callback for ImGUI

			switch (event.type)
			{
				case SDL_EVENT_QUIT:
					OnQuitEvent();
					break;
				case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
					OnWindowResizeEvent();
					break;
				case SDL_EVENT_KEY_DOWN:
					OnKeyEvent(event.key.scancode, EKeyAction::Pressed);
					break;
				case SDL_EVENT_KEY_UP:
					OnKeyEvent(event.key.scancode, EKeyAction::Released);
					break;
				case SDL_EVENT_MOUSE_MOTION:
					OnMouseCursorEvent(event.motion.x, event.motion.y);
					break;
				case SDL_EVENT_MOUSE_WHEEL:
					OnMouseScrollEvent(event.wheel.y);
					break;
				case SDL_EVENT_MOUSE_BUTTON_DOWN:
					OnMouseButtonEvent(event.button.button, EMouseButtonAction::Pressed);
					break;
				case SDL_EVENT_MOUSE_BUTTON_UP:
					OnMouseButtonEvent(event.button.button, EMouseButtonAction::Released);
					break;
				default:
					break;
			}
		}
	}

	void Platform::ShowWindow() const
	{
		if (!SDL_ShowWindow(m_self->window))
			throw std::runtime_error("Cannot show SDL window: " + std::string(SDL_GetError()));
	}

	void Platform::SetWindowResized(const bool resized)
	{
		m_windowResized = resized;
	}

	bool Platform::IsWindowResized() const
	{
		return m_windowResized;
	}

	auto Platform::GetWindowSize() const -> std::pair<int32_t, int32_t>
	{
		int32_t width, height;
		SDL_GetWindowSizeInPixels(m_self->window, &width, &height);
		return {width, height};
	}

	void* Platform::GetWindow() const
	{
		return m_self->window;
	}

	bool Platform::IsRunning() const
	{
		return m_running;
	}

	void Platform::OnQuitEvent()
	{
		m_running = false;
	}

	void Platform::OnWindowResizeEvent()
	{
		m_windowResized = true;
	}

	void Platform::OnKeyEvent(int32_t key, EKeyAction action)
	{
		LegacyInputManager::UpdateKey(key, action);
	}

	void Platform::OnMouseCursorEvent(const float xPos, const float yPos)
	{
		LegacyInputManager::UpdateMousePosition(xPos, yPos);
	}

	void Platform::OnMouseScrollEvent(const float yOffset)
	{
		LegacyInputManager::UpdateMouseScroll(yOffset);
	}

	void Platform::OnMouseButtonEvent(int32_t button, EMouseButtonAction action)
	{
		LegacyInputManager::UpdateMouseButton(button, action);
	}
}
