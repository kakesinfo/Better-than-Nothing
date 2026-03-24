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

#include "Platform/WindowBuilder.hpp"
#include "Platform/Window.hpp"

namespace BetterThanNothing
{

	WindowBuilder::WindowBuilder(const std::string& title, const uint32_t width, const uint32_t height)
	{
		m_windowParams.title = title;
		m_windowParams.width = width;
		m_windowParams.height = height;
	}

	WindowBuilder::~WindowBuilder() = default;

	WindowBuilder& WindowBuilder::SetTitle(const std::string& title)
	{
		m_windowParams.title = title;
		return *this;
	}

	WindowBuilder& WindowBuilder::SetSize(const uint32_t width, const uint32_t height)
	{
		m_windowParams.width = width;
		m_windowParams.height = height;
		return *this;
	}

	WindowBuilder& WindowBuilder::SeFullscreen(const bool fullscreen)
	{
		m_windowParams.fullscreen = fullscreen;
		return *this;
	}

	WindowBuilder& WindowBuilder::SetResizable(const bool resizable)
	{
		m_windowParams.resizable = resizable;
		return *this;
	}

	std::unique_ptr<Window> WindowBuilder::Build() const
	{
		if (m_windowParams.title.empty())
			throw WindowBuilderError("CWindow title is not set");

		if (m_windowParams.width == 0 || m_windowParams.height == 0)
			throw WindowBuilderError("CWindow size is not set");

		return std::make_unique<
			Window>(m_windowParams.title, m_windowParams.width, m_windowParams.height, m_windowParams.fullscreen, m_windowParams.resizable);
	}
}
