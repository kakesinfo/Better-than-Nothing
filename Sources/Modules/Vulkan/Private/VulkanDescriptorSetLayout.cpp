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
	VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& layoutBindings, VulkanContext* context) : m_context(context)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<std::uint32_t>(layoutBindings.size());
		layoutInfo.pBindings = layoutBindings.data();

		if (vkCreateDescriptorSetLayout(context->vkDevice->Handle(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
			throw VulkanDescriptorSetLayoutError("Failed to create descriptor set layout!");
	}

	VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
	{
		if (m_descriptorSetLayout != VK_NULL_HANDLE)
			vkDestroyDescriptorSetLayout(m_context->vkDevice->Handle(), m_descriptorSetLayout, nullptr);
	}

	VkDescriptorSetLayout VulkanDescriptorSetLayout::Handle() const
	{
		return m_descriptorSetLayout;
	}
}