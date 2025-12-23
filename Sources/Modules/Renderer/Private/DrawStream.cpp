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

#include "Module_Renderer/DrawStream.hpp"

namespace BetterThanNothing
{
	DrawStream::DrawStream() = default;
	DrawStream::~DrawStream() = default;

	void DrawStream::Draw(const DrawCommand& command)
	{
		m_commands.push_back(command);
	}

	DrawCommand& DrawStream::operator[](const std::uint32_t index)
	{
		return m_commands.at(index);
	}

	const DrawCommand& DrawStream::operator[](const std::uint32_t index) const
	{
		return m_commands.at(index);
	}

	std::uint32_t DrawStream::Size() const
	{
		return m_commands.size();
	}
}
