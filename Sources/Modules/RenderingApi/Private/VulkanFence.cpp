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
	VulkanFence::VulkanFence(VulkanContext* context) : m_context(context)
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		auto device = m_context->vkDevice->Handle();
		if (vkCreateFence(device, &fenceInfo, nullptr, &m_fence) != VK_SUCCESS)
			throw std::runtime_error("failed to create fence!");
	}

	VulkanFence::~VulkanFence()
	{
		if (m_fence != VK_NULL_HANDLE)
		{
			auto device = m_context->vkDevice->Handle();
			vkDestroyFence(device, m_fence, nullptr);
		}
	}

	void VulkanFence::Wait() const
	{
		auto device = m_context->vkDevice->Handle();
		vkWaitForFences(device, 1, &m_fence, VK_TRUE, UINT64_MAX);
	}

	void VulkanFence::Reset() const
	{
		auto device = m_context->vkDevice->Handle();
		vkResetFences(device, 1, &m_fence);
	}

	VkFence VulkanFence::Handle() const
	{
		return m_fence;
	}
}
