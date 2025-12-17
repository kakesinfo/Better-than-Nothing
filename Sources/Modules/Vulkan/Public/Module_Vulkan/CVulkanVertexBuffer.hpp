#pragma once

#include "CVulkanBuffer.hpp"

namespace BetterThanNothing
{
	struct VulkanContext;

	class CVulkanVertexBuffer: public CVulkanBuffer
	{
	public:
		CVulkanVertexBuffer(VulkanContext* context, VkDeviceSize size, const void* data = nullptr);
	};
}

