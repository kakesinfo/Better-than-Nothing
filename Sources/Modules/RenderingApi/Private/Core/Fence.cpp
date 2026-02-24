/*
 *	Better than Nothing
 *	Copyright (C) 2026 kakesinfo
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

namespace BetterThanNothing::RenderingApi
{
	Fence::Fence(const FenceCreateInfo& createInfo): m_device(createInfo.device)
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		if (createInfo.isSignaled)
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateFence(m_device->Handle(), &fenceInfo, nullptr, &m_fence) != VK_SUCCESS)
			throw std::runtime_error("failed to create fence!");
	}

	Fence::~Fence()
	{
		if (m_fence != VK_NULL_HANDLE)
			vkDestroyFence(m_device->Handle(), m_fence, nullptr);
	}

	std::unique_ptr<Fence> Fence::Create(const FenceCreateInfo& createInfo)
	{
		return std::make_unique<Fence>(createInfo);
	}

	void Fence::Wait() const
	{
		vkWaitForFences(m_device->Handle(), 1, &m_fence, VK_TRUE, UINT64_MAX);
	}

	void Fence::Reset()
	{
		vkResetFences(m_device->Handle(), 1, &m_fence);
	}

	VkFence Fence::Handle() const
	{
		return m_fence;
	}
}
