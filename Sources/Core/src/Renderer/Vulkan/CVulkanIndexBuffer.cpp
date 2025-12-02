//
// Created by COEZARAX on 02/12/2025.
//

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanIndexBuffer::CVulkanIndexBuffer(CVulkanContext* context, VkDeviceSize size, const void* data)
		: CVulkanBuffer_New(context, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, 0)
	{
		if (data)
		{
			const CVulkanStagingBuffer stagingBuffer(context, size, data);

			CVulkanCommandBuffer::SingleTimeCommands([&](const std::unique_ptr<CVulkanCommandBuffer>& commandBuffer) {
				commandBuffer->CopyBuffer(stagingBuffer.Handle(), m_buffer, size);
			}, context);
		}
	}
}

