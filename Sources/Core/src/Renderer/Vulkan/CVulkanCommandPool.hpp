#pragma once

#include "CVulkanContext.hpp"

namespace BetterThanNothing
{
	struct CVulkanContext;

	class CVulkanCommandPool
	{
	private:
		CVulkanContext* m_context = nullptr;
		VkCommandPool m_commandPool = VK_NULL_HANDLE;

	public:
		explicit CVulkanCommandPool(CVulkanContext* context);
		~CVulkanCommandPool();

		CVulkanCommandPool(const CVulkanCommandPool&) = delete;
		CVulkanCommandPool& operator=(const CVulkanCommandPool&) = delete;
		CVulkanCommandPool(CVulkanCommandPool&& other) = delete;
		CVulkanCommandPool& operator=(CVulkanCommandPool&& other) = delete;

		[[nodiscard]]
		VkCommandPool Handle() const;
	};
}
