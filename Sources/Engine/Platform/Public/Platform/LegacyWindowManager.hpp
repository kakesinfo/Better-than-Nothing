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

#include <memory>
#include <stdexcept>

namespace BetterThanNothing
{
	class Window;

	class LegacyWindowManagerError final : public std::runtime_error
	{
	public:
		explicit LegacyWindowManagerError(const std::string& message) : std::runtime_error(message) {}
	};

	struct LegacyWindowManagerParams
	{
		std::string title;
		uint32_t	width = 0;
		uint32_t	height = 0;
		bool		fullscreen = false;
		bool		resizable = false;
	};

	class LegacyWindowManager
	{
	public:
		LegacyWindowManager() = default;
		~LegacyWindowManager() = default;

		static void						Initialize(const LegacyWindowManagerParams& params);
		static std::unique_ptr<Window>& GetWindow();
		static void						Shutdown();

	private:
		static std::unique_ptr<Window> m_window;
	};
};
