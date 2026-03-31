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

#include "RenderingApi/Core/PipelineLayout.hpp"

#include "RenderingApi/Core/DescriptorSetLayout.hpp"
#include "RenderingApi/Core/Device.hpp"

namespace BetterThanNothing::RenderingApi
{
	PipelineLayout::PipelineLayout(const PipelineLayoutCreateInfo& createInfo) : m_createInfo(createInfo), m_device(createInfo.device)
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(createInfo.descriptorLayouts.size());
		pipelineLayoutInfo.pSetLayouts = createInfo.descriptorLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = createInfo.pushConstants.data();

		if (vkCreatePipelineLayout(m_device->Handle(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("Failed to create pipeline layout");
	}

	PipelineLayout::~PipelineLayout()
	{
		if (m_device && m_pipelineLayout)
			vkDestroyPipelineLayout(m_device->Handle(), m_pipelineLayout, nullptr);
	}

	VkPipelineLayout PipelineLayout::Handle() const
	{
		return m_pipelineLayout;
	}
}
