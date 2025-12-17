#pragma once

#include "CVulkanBuffer.hpp"

namespace BetterThanNothing
{
	struct VulkanContext;

	class CVulkanUniformBuffer final : public CVulkanBuffer
	{
	public:
		CVulkanUniformBuffer(VulkanContext* context, VkDeviceSize size);

		template <typename T>
		void Update(const T& uboData) {
			Write(&uboData, sizeof(T));
		}

		template <typename T>
		void Update(const T& uboData, const std::uint32_t offset, const std::uint32_t alignment) {
			Write(&uboData, sizeof(T), offset, alignment);
		}

		void* GetData() const;
	};
}
