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

#pragma once

#include <Common/EKey.hpp>
#include <Common/EMouseButton.hpp>
#include <memory>

namespace BetterThanNothing
{
	struct PlatformInternals;

	struct PlatformCreateInfo
	{
		std::string windowTitle;
		int32_t		windowWidth;
		int32_t		windowHeight;
		bool		windowFullscreen;
		bool		windowResizable;
	};

	class Platform
	{
	public:
		explicit Platform(const PlatformCreateInfo& createInfo);
		~Platform();

		static std::unique_ptr<Platform> Create(std::string windowTitle, int32_t windowWidth, int32_t windowHeight);

		void PollEvents();

		void				ShowWindow() const;
		void				SetWindowResized(bool resized);
		[[nodiscard]] bool	IsWindowResized() const;
		[[nodiscard]] auto	GetWindowSize() const -> std::pair<int32_t, int32_t>;
		[[nodiscard]] void* GetWindow() const;
		[[nodiscard]] bool	IsRunning() const;

	protected:
		void OnQuitEvent();
		void OnWindowResizeEvent();
		void OnKeyEvent(int32_t key, EKeyAction action);
		void OnMouseCursorEvent(float xPos, float yPos);
		void OnMouseScrollEvent(float yOffset);
		void OnMouseButtonEvent(int32_t button, EMouseButtonAction action);

	private:
		std::unique_ptr<PlatformInternals> m_self;
		const PlatformCreateInfo		   m_createInfo;
		bool							   m_running;
		int32_t							   m_windowWidth;
		int32_t							   m_windowHeight;
		bool							   m_windowResized;
	};
}
