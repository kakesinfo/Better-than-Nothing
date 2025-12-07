//
// Created by COEZARAX on 02/12/2025.
//

#pragma once

#include "CVulkanBuffer.hpp"

namespace BetterThanNothing
{
	class CVulkanUniformBuffer: public CVulkanBuffer
	{
	public:
		CVulkanUniformBuffer(VulkanContext* context, VkDeviceSize size);

		template <typename T>
		void Update(const T& uboData) {
			Write(&uboData, sizeof(T));
		}
	};
}
