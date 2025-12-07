//
// Created by COEZARAX on 02/12/2025.
//

#pragma once

#include "CVulkanBuffer.hpp"

namespace BetterThanNothing
{
	class CVulkanUniformBuffer final : public CVulkanBuffer
	{
	public:
		CVulkanUniformBuffer(VulkanContext* context, VkDeviceSize size);

		template <typename T>
		void Update(const T& uboData) {
			Write(&uboData, sizeof(T));
		}

		template <typename T>
		void Update(const T& uboData, const uint32 offset, const uint32 alignment) {
			Write(&uboData, sizeof(T), offset, alignment);
		}

		void* GetData() const;
	};
}
