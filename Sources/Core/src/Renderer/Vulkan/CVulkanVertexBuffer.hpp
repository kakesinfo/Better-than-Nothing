//
// Created by COEZARAX on 02/12/2025.
//

#pragma once

#include "CVulkanBuffer.hpp"

namespace BetterThanNothing
{
	class CVulkanVertexBuffer: public CVulkanBuffer
	{
	public:
		CVulkanVertexBuffer(VulkanContext* context, VkDeviceSize size, const void* data = nullptr);
	};
}

