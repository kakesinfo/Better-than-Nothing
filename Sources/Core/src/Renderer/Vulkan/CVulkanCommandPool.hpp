#pragma once

#include "VulkanContext.hpp"

namespace BetterThanNothing
{
	struct VulkanContext;

	class CVulkanCommandPool
	{
	public:
		explicit CVulkanCommandPool(VulkanContext* context);
		~CVulkanCommandPool();

		CVulkanCommandPool(const CVulkanCommandPool&) = delete;
		CVulkanCommandPool& operator=(const CVulkanCommandPool&) = delete;
		CVulkanCommandPool(CVulkanCommandPool&& other) = delete;
		CVulkanCommandPool& operator=(CVulkanCommandPool&& other) = delete;

		[[nodiscard]]
		VkCommandPool Handle() const;

	private:
		VulkanContext* m_context = nullptr;
		VkCommandPool m_commandPool = VK_NULL_HANDLE;
	};
}
