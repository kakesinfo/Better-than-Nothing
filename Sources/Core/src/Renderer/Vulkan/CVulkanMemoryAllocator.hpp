//
// Created by acoezard on 11/19/24.
//

#pragma once

#include "vk_mem_alloc.h"

namespace BetterThanNothing
{
	struct VulkanContext;

	class CVulkanMemoryAllocator
	{
	public:
		explicit CVulkanMemoryAllocator(VulkanContext* pContext);
		~CVulkanMemoryAllocator();

		VkResult CreateBuffer(const VkBufferCreateInfo* bufferCreateInfo, const VmaAllocationCreateInfo* allocCreateInfo, VkBuffer* buffer, VmaAllocation* allocation, VmaAllocationInfo* allocationInfo) const;
		void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation) const;

		VkResult CreateImage(const VkImageCreateInfo* imageCreateInfo, const VmaAllocationCreateInfo* allocCreateInfo, VkImage* image, VmaAllocation* allocation, VmaAllocationInfo* allocationInfo) const;
		void DestroyImage(VkImage image, VmaAllocation allocation) const;

		VkResult MapMemory(VmaAllocation allocation, void** data) const;
		void UnmapMemory(VmaAllocation allocation) const;

		[[nodiscard]]
		VmaAllocator Handle() const;

	private:
		VmaAllocator m_allocator = VK_NULL_HANDLE;
	};
};
