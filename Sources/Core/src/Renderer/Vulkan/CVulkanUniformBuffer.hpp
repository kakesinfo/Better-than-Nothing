//
// Created by COEZARAX on 02/12/2025.
//

#pragma once

#include "CVulkanBuffer_New.hpp"

namespace BetterThanNothing
{
	class CVulkanUniformBuffer: public CVulkanBuffer_New
	{
	public:
		CVulkanUniformBuffer(CVulkanContext* context, VkDeviceSize size);

		template <typename T>
		void Update(const T& uboData) {
			Write(&uboData, sizeof(T));
		}
	};
}
