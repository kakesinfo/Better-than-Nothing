//
// Created by COEZARAX on 02/12/2025.
//

#pragma once

#include "CVulkanBuffer.hpp"

namespace BetterThanNothing
{
	class CVulkanStagingBuffer: public CVulkanBuffer
	{
	public:
		CVulkanStagingBuffer(CVulkanContext* context, VkDeviceSize size, const void* data = nullptr);
	};
}
