//
// Created by COEZARAX on 02/12/2025.
//

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanUniformBuffer::CVulkanUniformBuffer(VulkanContext* context, VkDeviceSize size)
		: CVulkanBuffer(context, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VMA_ALLOCATION_CREATE_MAPPED_BIT) {}

	void *CVulkanUniformBuffer::GetData() const
	{
		return m_allocInfo.pMappedData;
	}

}
