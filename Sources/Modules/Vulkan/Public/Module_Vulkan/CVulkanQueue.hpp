#pragma once

namespace BetterThanNothing
{
	class CVulkanQueue
	{
	public:
		explicit CVulkanQueue(VkQueue queue);
		~CVulkanQueue() = default;

		CVulkanQueue(const CVulkanQueue&) = delete;
		CVulkanQueue& operator=(const CVulkanQueue&) = delete;
		CVulkanQueue(CVulkanQueue&& other) = delete;
		CVulkanQueue& operator=(CVulkanQueue&& other) = delete;

		[[nodiscard]]
		VkQueue Handle() const;

	private:
		VkQueue m_queue = VK_NULL_HANDLE;
	};
} // BetterThanNothing