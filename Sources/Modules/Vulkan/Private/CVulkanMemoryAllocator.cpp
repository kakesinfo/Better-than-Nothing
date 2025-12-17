#include "module_vulkan.hpp"

#include <Module_Base/CLogger.hpp>

namespace BetterThanNothing
{
	CVulkanMemoryAllocator::CVulkanMemoryAllocator(VulkanContext* pContext)
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

		CLogger::Log<Success>("Vulkan memory allocator (Vma): ok");
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

	VkResult CVulkanMemoryAllocator::CreateImage(
		const VkImageCreateInfo* imageCreateInfo,
		const VmaAllocationCreateInfo* allocCreateInfo,
		VkImage* image,
		VmaAllocation* allocation,
		VmaAllocationInfo* allocationInfo
	) const {
		return vmaCreateImage(m_allocator, imageCreateInfo, allocCreateInfo, image, allocation, allocationInfo);
	}

	void CVulkanMemoryAllocator::DestroyImage(VkImage image, VmaAllocation allocation) const
	{
		vmaDestroyImage(m_allocator, image, allocation);
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