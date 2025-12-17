//
// Created by COEZARAX on 02/12/2025.
//

#pragma once

#include "Module_Vulkan/CVulkanBuffer.hpp"

namespace BetterThanNothing
{
	struct VulkanContext;

	class CVulkanIndexBuffer: public CVulkanBuffer
	{
	public:
		CVulkanIndexBuffer(VulkanContext* context, VkDeviceSize size, const void* data = nullptr);
	};
}
