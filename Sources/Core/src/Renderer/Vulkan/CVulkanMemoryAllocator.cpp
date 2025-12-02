//
// Created by acoezard on 11/19/24.
//

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanMemoryAllocator::CVulkanMemoryAllocator(CVulkanContext* pContext)
	{
		VmaVulkanFunctions vulkanFunctions = {};
		vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
		vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo allocatorCreateInfo = {};
		allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT; // Ext: VK_EXT_memory_budget
		allocatorCreateInfo.vulkanApiVersion = pContext->vkSettings.apiVersion;
		allocatorCreateInfo.physicalDevice = pContext->vkDevice->PhysicalHandle();
		allocatorCreateInfo.device = pContext->vkDevice->Handle();
		allocatorCreateInfo.instance = pContext->vkInstance->Handle();
		allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

		if (vmaCreateAllocator(&allocatorCreateInfo, &m_allocator) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan allocator");

		LOG_SUCCESS("Vulkan memory allocator (Vma): ok");
	}

	/**
	 * @deprecated Old implementation for CVulkanBuffer_Old
	 */
	VkResult CVulkanMemoryAllocator::CreateBuffer(
		const VkBufferCreateInfo* bufferCreateInfo,
		const VkMemoryPropertyFlags properties,
		VkBuffer* buffer,
		VmaAllocation* allocation,
		VmaAllocationInfo* allocationInfo
	) const {
		return CreateBuffer(bufferCreateInfo, properties, 0, buffer, allocation, allocationInfo);
	}

	/**
	 * @deprecated Old implementation for CVulkanBuffer_Old
	 */
	VkResult CVulkanMemoryAllocator::CreateBuffer(
		const VkBufferCreateInfo* bufferCreateInfo,
		const VkMemoryPropertyFlags properties,
		const VmaAllocationCreateFlags flags, // Use: VMA_ALLOCATION_CREATE_MAPPED_BIT; to map the memory automatically
		VkBuffer* buffer,
		VmaAllocation* allocation,
		VmaAllocationInfo* allocationInfo
	) const {
		VmaAllocationCreateInfo allocInfo = {};

		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocInfo.requiredFlags = properties;
		allocInfo.flags = flags;

		if (flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) {
			allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		}

		return vmaCreateBuffer(m_allocator, bufferCreateInfo, &allocInfo, buffer, allocation, allocationInfo);
	}

	VkResult CVulkanMemoryAllocator::CreateBuffer(
		const VkBufferCreateInfo* bufferCreateInfo,
		const VmaAllocationCreateInfo* allocCreateInfo,
		VkBuffer* buffer,
		VmaAllocation* allocation,
		VmaAllocationInfo* allocationInfo
	) const {
		return vmaCreateBuffer(m_allocator, bufferCreateInfo, allocCreateInfo, buffer, allocation, allocationInfo);
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