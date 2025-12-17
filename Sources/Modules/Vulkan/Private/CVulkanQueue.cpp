#include "module_vulkan.hpp"

namespace BetterThanNothing
{
	CVulkanQueue::CVulkanQueue(VkQueue queue): m_queue(queue) {}

	VkQueue CVulkanQueue::Handle() const
	{
		return m_queue;
	}
}