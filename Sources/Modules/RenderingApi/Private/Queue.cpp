/*
 *	Better than Nothing
 *	Copyright (C) 2025 kakesinfo
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 * 	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "RenderingApi.hpp"

namespace BetterThanNothing
{
	Queue::Queue(const QueueCreateInfo& createInfo) : m_createInfo(createInfo) {}
	Queue::~Queue() = default;

	std::unique_ptr<Queue> Queue::Create(const QueueCreateInfo& createInfo)
	{
		return std::make_unique<Queue>(createInfo);
	}

	VkResult Queue::Submit(const uint32_t submitCount, const VkSubmitInfo* submits, VkFence fence) const
	{
		std::lock_guard lock(m_mutex);
		return vkQueueSubmit(m_createInfo.handle, submitCount, submits, fence);
	}

	VkResult Queue::Present(const VkPresentInfoKHR* presentInfo) const
	{
		std::lock_guard lock(m_mutex);
		return vkQueuePresentKHR(m_createInfo.handle, presentInfo);
	}

	VkResult Queue::WaitIdle() const
	{
		std::lock_guard lock(m_mutex);
		return vkQueueWaitIdle(m_createInfo.handle);
	}

	std::uint32_t Queue::GetFamilyIndex() const
	{
		return m_createInfo.familyIndex;
	}

	float Queue::GetPriority() const
	{
		return m_createInfo.priority;
	}

	VkQueue Queue::Handle() const
	{
		return m_createInfo.handle;
	}
}
