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

namespace BetterThanNothing
{
	class Window;

	class WindowBuilderError final : public std::runtime_error
	{
	public:
		explicit WindowBuilderError(const std::string& message) : std::runtime_error(message) {}
	};

	struct WindowBuilderParams
	{
		std::string	  title;
		std::uint32_t width = 0;
		std::uint32_t height = 0;
		bool		  fullscreen = false;
		bool		  resizable = false;
	};

	class WindowBuilder
	{
	public:
		explicit WindowBuilder(const std::string& title, std::uint32_t width, std::uint32_t height);
		~WindowBuilder();

		WindowBuilder& SetTitle(const std::string& title);
		WindowBuilder& SetSize(std::uint32_t width, std::uint32_t height);
		WindowBuilder& SeFullscreen(bool fullscreen);
		WindowBuilder& SetResizable(bool resizable);

		std::unique_ptr<Window> Build() const;

	private:
		WindowBuilderParams m_windowParams{};
	};
}
