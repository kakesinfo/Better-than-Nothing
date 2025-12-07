//
// Created by COEZARAX on 02/12/2025.
//

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanStagingBuffer::CVulkanStagingBuffer(VulkanContext *context, VkDeviceSize size, const void *data)
		: CVulkanBuffer(context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VMA_ALLOCATION_CREATE_MAPPED_BIT)
	{
		if (data) Write(data, size);
	}
}
