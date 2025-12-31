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

#pragma once

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace BetterThanNothing
{
	struct VulkanContext;

	class CVulkanBufferError final : public std::runtime_error
	{
	public:
		explicit CVulkanBufferError(const std::string& message) : std::runtime_error(message) {}
	};

	class VulkanBuffer
	{
	public:
		VulkanBuffer(VulkanContext* context, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
					 VmaAllocationCreateFlags vmaFlags);
		virtual ~VulkanBuffer();

		VulkanBuffer(const VulkanBuffer&) = delete;
		VulkanBuffer& operator=(const VulkanBuffer&) = delete;

		[[nodiscard]] VkBuffer	   Handle() const;
		[[nodiscard]] VkDeviceSize Size() const;

		void Write(const void* data, VkDeviceSize size, VkDeviceSize offset = 0, VkDeviceSize alignment = 1) const;

	protected:
		VulkanContext*	  m_context;
		VkBuffer		  m_buffer = VK_NULL_HANDLE;
		VmaAllocation	  m_allocation = VK_NULL_HANDLE;
		VmaAllocationInfo m_allocInfo = {};
		VkDeviceSize	  m_size;
	};
}
