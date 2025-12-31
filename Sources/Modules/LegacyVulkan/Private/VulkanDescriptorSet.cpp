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

#include "module_vulkan.hpp"

namespace BetterThanNothing
{
	VulkanDescriptorSet::VulkanDescriptorSet(const std::unique_ptr<VulkanDescriptorSetLayout>& layout,
											 VulkanContext*									   context)
		: m_context(context)
	{
		auto handle = layout->Handle();

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_context->vkDescriptorPool->Handle();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &handle;

		if (vkAllocateDescriptorSets(m_context->vkDevice->Handle(), &allocInfo, &m_descriptorSet) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate descriptor set");
	}

	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
		// Note: Descriptor sets are automatically freed when the descriptor pool is destroyed.
	}

	VkDescriptorSet VulkanDescriptorSet::Handle() const
	{
		return m_descriptorSet;
	}
} // BetterThanNothing