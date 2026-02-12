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
	VulkanSemaphore::VulkanSemaphore(VulkanContext* context) : m_context(context)
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.flags = 0;

		auto device = m_context->vkDevice->Handle();
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_semaphore) != VK_SUCCESS)
			throw std::runtime_error("failed to create semaphore!");
	}

	VulkanSemaphore::~VulkanSemaphore()
	{
		if (m_semaphore != VK_NULL_HANDLE)
		{
			auto device = m_context->vkDevice->Handle();
			vkDestroySemaphore(device, m_semaphore, nullptr);
		}
	}

	VkSemaphore VulkanSemaphore::Handle() const
	{
		return m_semaphore;
	}
}
