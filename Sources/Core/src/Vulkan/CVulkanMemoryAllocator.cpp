//
// Created by acoezard on 11/19/24.
//

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanMemoryAllocator::CVulkanMemoryAllocator(CApplicationContext* pContext)
	{
		VmaVulkanFunctions vulkanFunctions = {};
		vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
		vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo allocatorCreateInfo = {};
		allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
		allocatorCreateInfo.vulkanApiVersion = pContext->GetVulkanApiVersion();
		allocatorCreateInfo.physicalDevice = pContext->GetVulkanDevice()->PhysicalHandle();
		allocatorCreateInfo.device = pContext->GetVulkanDevice()->LogicalHandle();
		allocatorCreateInfo.instance = pContext->GetVulkanInstance()->Handle();
		allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

		if (vmaCreateAllocator(&allocatorCreateInfo, &m_allocator) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan allocator");

		LOG_SUCCESS("Vulkan memory allocator (Vma): ok");
	}

	VkResult CVulkanMemoryAllocator::CreateBuffer(const VkBufferCreateInfo* bufferCreateInfo, VkMemoryPropertyFlags properties,
												  VkBuffer* buffer, VmaAllocation* allocation, VmaAllocationInfo* allocationInfo) const
	{
		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_UNKNOWN;
		allocInfo.requiredFlags = properties;
		// Use: allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT; to map the memory automatically

		return vmaCreateBuffer(m_allocator, bufferCreateInfo, &allocInfo, buffer, allocation, allocationInfo);
	}

	void CVulkanMemoryAllocator::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation) const
	{
		vmaDestroyBuffer(m_allocator, buffer, allocation);
	}

	VkResult CVulkanMemoryAllocator::MapMemory(VmaAllocation allocation, void** data) const
	{
		return vmaMapMemory(m_allocator, allocation, data);
	}

	void CVulkanMemoryAllocator::UnmapMemory(VmaAllocation allocation) const
	{
		return vmaUnmapMemory(m_allocator, allocation);
	}

	CVulkanMemoryAllocator::~CVulkanMemoryAllocator()
	{
		 vmaDestroyAllocator(m_allocator);
	}

	VmaAllocator CVulkanMemoryAllocator::Handle() const
	{
		return m_allocator;
	}
}