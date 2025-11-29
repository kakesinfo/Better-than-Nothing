#pragma once

namespace BetterThanNothing
{
	struct CVulkanContext;

	class CVulkanSemaphore
	{
	private:
		CVulkanContext* m_context = nullptr;
		VkSemaphore m_semaphore = VK_NULL_HANDLE;

	public:
		explicit CVulkanSemaphore(CVulkanContext* context);
		~CVulkanSemaphore();

		[[nodiscard]]
		VkSemaphore Handle() const;
	};
}
