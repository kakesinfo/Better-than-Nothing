#pragma once

namespace BetterThanNothing
{
	struct VulkanContext;

	class CVulkanSemaphore
	{
	public:
		explicit CVulkanSemaphore(VulkanContext* context);
		~CVulkanSemaphore();

		[[nodiscard]]
		VkSemaphore Handle() const;

	private:
		VulkanContext* m_context = nullptr;
		VkSemaphore m_semaphore = VK_NULL_HANDLE;
	};
}
