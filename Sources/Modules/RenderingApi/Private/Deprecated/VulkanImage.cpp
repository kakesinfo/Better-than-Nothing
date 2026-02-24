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

namespace BetterThanNothing
{
	VulkanImage::VulkanImage(VulkanContext* context, const VkImageCreateInfo* imageCreateInfo) : m_context(context)
	{
		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		allocCreateInfo.priority = 1.0f;

		if (m_context->allocator->CreateImage(imageCreateInfo, &allocCreateInfo, &m_image, &m_allocation,
													  &m_allocationInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan image.");
	}

	VulkanImage::~VulkanImage()
	{
		if (m_image && m_allocation)
			m_context->allocator->DestroyImage(m_image, m_allocation);
	}

	VkImage VulkanImage::Handle() const
	{
		return m_image;
	}
}
