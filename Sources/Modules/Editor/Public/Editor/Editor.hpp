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
#include <vector>

namespace BetterThanNothing
{
	class Window;
	class RenderingDevice;
	class Panel;

	namespace RenderingApi
	{
		class CommandBuffer;
	}

	struct EditorInternals;

	class Editor
	{
	public:
		Editor(Window* window, RenderingDevice* renderingDevice);
		~Editor();

		static std::unique_ptr<Editor> Create(Window* window, RenderingDevice* renderingDevice);

		template<typename T>
		void AddPanel(std::string_view name)
		{
			m_panels.push_back(std::make_unique<T>(name));
		}

		void RenderPanels() const;
		void RenderDrawData(const RenderingApi::CommandBuffer* commandBuffer) const;

	private:
		Window*							 m_window;
		RenderingDevice*				 m_renderingDevice;
		std::unique_ptr<EditorInternals> m_internals;

		std::vector<std::unique_ptr<Panel>> m_panels;
	};
}
