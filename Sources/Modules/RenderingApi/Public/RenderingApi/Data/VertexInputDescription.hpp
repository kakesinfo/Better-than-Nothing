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

#include <vector>
#include <vulkan/vulkan.h>

namespace BetterThanNothing
{
	struct VertexInputDescription
	{
		std::vector<VkVertexInputBindingDescription>   bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;

		[[nodiscard]] VkPipelineVertexInputStateCreateInfo GetCreateInfo() const
		{
			VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

			vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
			vertexInputInfo.pVertexBindingDescriptions = bindings.data();

			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
			vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

			return vertexInputInfo;
		}
	};
}
