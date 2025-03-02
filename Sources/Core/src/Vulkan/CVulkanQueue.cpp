//
// Created by acoezard on 11/18/24.
//

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanQueue::CVulkanQueue(VkQueue queue): m_queue(queue) {}

	VkQueue CVulkanQueue::Handle() const
	{
		return m_queue;
	}
}