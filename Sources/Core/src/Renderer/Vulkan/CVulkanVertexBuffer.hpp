//
// Created by COEZARAX on 02/12/2025.
//

#pragma once

#include "CVulkanBuffer_New.hpp"

namespace BetterThanNothing
{
	class CVulkanVertexBuffer: public CVulkanBuffer_New
	{
	public:
		CVulkanVertexBuffer(CVulkanContext* context, VkDeviceSize size, const void* data = nullptr);
	};
}

