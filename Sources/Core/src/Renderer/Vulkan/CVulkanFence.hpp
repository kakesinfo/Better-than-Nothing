#pragma once

namespace BetterThanNothing
{
	struct CVulkanContext;

	class CVulkanFence
	{
	private:
		CVulkanContext* m_context = nullptr;
		VkFence m_fence = VK_NULL_HANDLE;

	public:
		explicit CVulkanFence(CVulkanContext* context);
		~CVulkanFence();

		void Wait();
		void Reset();

		[[nodiscard]]
		VkFence Handle() const;
	};
}
