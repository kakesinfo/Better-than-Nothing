#pragma once

namespace BetterThanNothing
{
	class CApplicationContext;

	class CVulkanCommandPool
	{
	private:
		CApplicationContext* m_context = nullptr;
		VkCommandPool m_commandPool = VK_NULL_HANDLE;

	public:
		explicit CVulkanCommandPool(CApplicationContext* context);
		~CVulkanCommandPool();

		CVulkanCommandPool(const CVulkanCommandPool&) = delete;
		CVulkanCommandPool& operator=(const CVulkanCommandPool&) = delete;
		CVulkanCommandPool(CVulkanCommandPool&& other) = delete;
		CVulkanCommandPool& operator=(CVulkanCommandPool&& other) = delete;

		[[nodiscard]]
		VkCommandPool Handle() const;
	};
}
