#pragma once

namespace BetterThanNothing
{
	class CApplicationContext;

	class CVulkanSemaphore
	{
	private:
		CApplicationContext* m_context = nullptr;
		VkSemaphore m_semaphore = VK_NULL_HANDLE;

	public:
		explicit CVulkanSemaphore(CApplicationContext* context);
		~CVulkanSemaphore();

		[[nodiscard]]
		VkSemaphore Handle() const;
	};
}
