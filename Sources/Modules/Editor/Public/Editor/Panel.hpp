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

#include "Module_Math/Vector2.hpp"

#include <string>

namespace BetterThanNothing
{
	class Scene;

	class Panel
	{
	public:
		explicit Panel(const std::string_view name) : m_name(name) {}
		virtual ~Panel() = default;

		virtual void OnRender(Scene* scene) = 0;

		void							 SetOpen(const bool isOpen) { m_isOpen = isOpen; }
		[[nodiscard]] bool				 IsOpen() const { return m_isOpen; }
		[[nodiscard]] const std::string& GetName() const { return m_name; }

		void				SetSize(float width, float height) { m_size = {width, height}; }
		[[nodiscard]] bool	HasSize() const { return m_size.GetX() > 0 && m_size.GetY() > 0; }
		[[nodiscard]] float GetWidth() const { return m_size.GetX(); }
		[[nodiscard]] float GetHeight() const { return m_size.GetY(); }

		void				SetPosition(float x, float y) { m_position = {x, y}; }
		[[nodiscard]] bool	HasPosition() const { return m_position.GetX() > 0 && m_position.GetY() > 0; }
		[[nodiscard]] float GetX() const { return m_position.GetX(); }
		[[nodiscard]] float GetY() const { return m_position.GetY(); }

	private:
		std::string m_name;
		Vector2		m_size = {-1, -1};
		Vector2		m_position = {-1, -1};
		bool		m_isOpen = true;
	};
}
