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
	VulkanDescriptorPool::VulkanDescriptorPool(const std::vector<VkDescriptorPoolSize> &poolSizes, VulkanContext *context) : m_context(context)
	{
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = CalculateMaxSets(poolSizes); // 3 sets per frame (global + dynamic + sampler)

		if (vkCreateDescriptorPool(context->vkDevice->Handle(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
			throw std::runtime_error("Failed to create descriptor pool");
	}

	VulkanDescriptorPool::~VulkanDescriptorPool()
	{
		if (m_descriptorPool != VK_NULL_HANDLE)
			vkDestroyDescriptorPool(m_context->vkDevice->Handle(), m_descriptorPool, nullptr);
	}

	VkDescriptorPool VulkanDescriptorPool::Handle() const
	{
		return m_descriptorPool;
	}

	std::uint32_t VulkanDescriptorPool::CalculateMaxSets(const std::vector<VkDescriptorPoolSize> &poolSizes)
	{
		std::uint32_t totalDescriptors = 0;
		for (const auto &[_, descriptorCount]: poolSizes)
			totalDescriptors += descriptorCount;
		return totalDescriptors;
	}
}
