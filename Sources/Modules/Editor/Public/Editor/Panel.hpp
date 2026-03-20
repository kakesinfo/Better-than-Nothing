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

namespace BetterThanNothing
{
	class Panel
	{
	public:
		explicit Panel(const std::string_view name) : m_name(name) {}
		virtual ~Panel() = default;

		virtual void OnRender() = 0;

		void							 SetOpen(const bool isOpen) { m_isOpen = isOpen; }
		[[nodiscard]] bool				 IsOpen() const { return m_isOpen; }
		[[nodiscard]] const std::string& GetName() const { return m_name; }

	private:
		std::string m_name;
		bool		m_isOpen = true;
	};
}
