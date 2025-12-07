#pragma once

namespace BetterThanNothing
{
	struct VulkanContext;

	class CVulkanFence
	{
	public:
		explicit CVulkanFence(VulkanContext* context);
		~CVulkanFence();

		void Wait();
		void Reset();

		[[nodiscard]]
		VkFence Handle() const;

	private:
		VulkanContext* m_context = nullptr;
		VkFence m_fence = VK_NULL_HANDLE;
	};
}
