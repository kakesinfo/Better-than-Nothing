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

#include <SDL3/SDL.h>

namespace BetterThanNothing
{
	class Window
	{
		SDL_Window* m_window = nullptr;

		std::string m_title{};

		std::uint32_t m_width = 0;
		std::uint32_t m_height = 0;

		bool m_fullscreen = false;
		bool m_resizable = true;
		bool m_resized = false;

		bool m_shouldClose = false;

	public:
		explicit Window(std::string title, std::uint32_t width, std::uint32_t height, bool fullscreen = false,
						bool resizable = false);
		~Window();

		[[nodiscard]]
		SDL_Window* Handle() const;

		void PollEvents();

		void Show() const;

		[[nodiscard]]
		bool ShouldClose() const;

		[[nodiscard]]
		std::pair<std::uint32_t, std::uint32_t> GetFramebufferSize() const;

		[[nodiscard]]
		bool IsResized() const;
		void SetResized(bool resized);

	protected:
		void SetWidth(std::uint32_t width);
		void SetHeight(std::uint32_t height);

		static void ResizeCallback(Window* window, std::int32_t width, std::int32_t height);
		static void KeyCallback(std::int32_t key, EKeyAction action);
		static void MouseCursorCallback(double xpos, double ypos);
		static void MouseScrollCallback(double yoffset);
		static void MouseButtonCallback(std::int32_t button, EMouseButtonAction action);
	};
};
