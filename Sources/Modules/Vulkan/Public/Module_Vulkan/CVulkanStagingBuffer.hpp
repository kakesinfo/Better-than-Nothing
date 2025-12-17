#pragma once

#include "CVulkanBuffer.hpp"

namespace BetterThanNothing
{
	struct VulkanContext;

	class CVulkanStagingBuffer: public CVulkanBuffer
	{
	public:
		CVulkanStagingBuffer(VulkanContext* context, VkDeviceSize size, const void* data = nullptr);
	};
}
