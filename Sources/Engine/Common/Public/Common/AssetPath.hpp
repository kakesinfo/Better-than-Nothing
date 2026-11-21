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

#include <string>
#include <string_view>

namespace BetterThanNothing
{
	class AssetPath
	{
	public:
		/// Resolves a relative asset or shader path (e.g. "Shaders/main/main.vert", ".assets/sponza/sponza.obj",
		/// or legacy "../../../.assets/...") against the current working directory and parent directories up to the project root.
		[[nodiscard]] static std::string Resolve(std::string_view path);

		/// Optional explicit override for the root directory containing "Shaders/" and ".assets/".
		static void SetProjectRoot(std::string_view rootPath);
		[[nodiscard]] static std::string GetProjectRoot();

	private:
		static std::string s_projectRoot;
	};
}
