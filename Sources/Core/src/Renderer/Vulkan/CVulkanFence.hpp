#pragma once

namespace BetterThanNothing
{
	class CApplicationContext;

	class CVulkanFence
	{
	private:
		CApplicationContext* m_context = nullptr;
		VkFence m_fence = VK_NULL_HANDLE;

	public:
		explicit CVulkanFence(CApplicationContext* context);
		~CVulkanFence();

		void Wait();
		void Reset();

		[[nodiscard]]
		VkFence Handle() const;
	};
}
