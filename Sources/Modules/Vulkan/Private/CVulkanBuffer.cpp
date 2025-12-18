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

#include "module_vulkan.hpp"

namespace BetterThanNothing
{
	CVulkanBuffer::CVulkanBuffer(
		VulkanContext*					context,
		const VkDeviceSize				size,
		const VkBufferUsageFlags		usage,
		const VmaMemoryUsage			memoryUsage,
		const VmaAllocationCreateFlags	vmaFlags
	) : m_context(context), m_size(size) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = memoryUsage;
		allocCreateInfo.flags = vmaFlags;

		if (m_context->vkMemoryAllocator->CreateBuffer(&bufferInfo, &allocCreateInfo, &m_buffer, &m_allocation, &m_allocInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan buffer.");
	}

	CVulkanBuffer::~CVulkanBuffer()
	{
		if (m_buffer && m_allocation)
			m_context->vkMemoryAllocator->DestroyBuffer(m_buffer, m_allocation);
	}

	void CVulkanBuffer::Write(const void* data, const VkDeviceSize size, const VkDeviceSize offset, const VkDeviceSize alignment) const
	{
		if (!m_allocInfo.pMappedData)
			throw CVulkanBufferError("Cannot write to unmapped buffer");

		if (size + offset > m_size)
			throw CVulkanBufferError("Buffer overflow: Update size exceeds buffer size");

		char* base = static_cast<char*>(m_allocInfo.pMappedData);
		char* destAddress = base + offset * alignment;

		std::memcpy(destAddress, data, size);
	}

	VkBuffer CVulkanBuffer::Handle() const
	{
		return m_buffer;
	}

	VkDeviceSize CVulkanBuffer::Size() const
	{
		return m_size;
	}
}