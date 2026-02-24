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

#include "IRenderPass.hpp"

#include <memory>
#include <vector>

namespace BetterThanNothing
{
	class RenderList;
	struct VulkanContext;
	struct CameraData;
	class DrawStreamView;
	class RenderingDevice;

	class RenderGraph
	{
	public:
		explicit RenderGraph(VulkanContext* context, RenderingDevice* renderingDevice);
		~RenderGraph();

		template <typename T>
		void AddRenderPass()
		{
			static_assert(std::is_base_of_v<IRenderPass, T>, "T must inherit from IRenderPass");
			m_renderPasses.push_back(std::make_unique<T>(m_context, m_renderingDevice));
		}

		void Execute(const RenderPassContext& context, const DrawStreamView& drawStream) const;

	private:
		VulkanContext*							  m_context;
		RenderingDevice*						  m_renderingDevice;
		std::vector<std::unique_ptr<IRenderPass>> m_renderPasses;
	};
}
