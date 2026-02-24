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

#pragma once

#include <memory>
#include <vma/vk_mem_alloc.h>

namespace BetterThanNothing::RenderingApi
{
	class Instance;
	class Device;

	struct AllocatorCreateInfo
	{
		Instance* instance;
		Device*	  device;
	};

	class Allocator
	{
	public:
		explicit Allocator(const AllocatorCreateInfo& createInfo);
		~Allocator();

		Allocator(const Allocator&) = delete;
		Allocator& operator=(const Allocator&) = delete;
		Allocator(Allocator&& other) = delete;
		Allocator& operator=(Allocator&& other) = delete;

		[[nodiscard]] static std::unique_ptr<Allocator> Create(const AllocatorCreateInfo& createInfo);

		[[nodiscard]] VkResult CreateBuffer(const VkBufferCreateInfo* bufferCreateInfo, const VmaAllocationCreateInfo* allocCreateInfo, VkBuffer* buffer, VmaAllocation* allocation, VmaAllocationInfo* allocationInfo) const;
		void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation) const;
		[[nodiscard]] VkResult CreateImage(const VkImageCreateInfo* imageCreateInfo, const VmaAllocationCreateInfo* allocCreateInfo, VkImage* image, VmaAllocation* allocation, VmaAllocationInfo* allocationInfo) const;
		void				   DestroyImage(VkImage image, VmaAllocation allocation) const;
		[[nodiscard]] VkResult MapMemory(VmaAllocation allocation, void** data) const;
		void				   UnmapMemory(VmaAllocation allocation) const;
		void				   FlushAllocation(VmaAllocation allocation, VkDeviceSize offset, VkDeviceSize size) const;

		[[nodiscard]] VmaAllocator Handle() const;

	private:
		AllocatorCreateInfo m_createInfo;
		VmaAllocator		m_allocator = VK_NULL_HANDLE;
	};
};
