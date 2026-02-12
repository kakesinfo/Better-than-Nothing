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

#include <Module_Base/Logger.hpp>

namespace BetterThanNothing
{
	Allocator::Allocator(VulkanContext* context)
	{
		VmaVulkanFunctions vulkanFunctions = {};
		vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
		vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo allocatorCreateInfo = {};
		allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT; // Ext: VK_EXT_memory_budget
		allocatorCreateInfo.vulkanApiVersion = context->vkSettings.apiVersion;
		allocatorCreateInfo.physicalDevice = context->vkDevice->PhysicalHandle();
		allocatorCreateInfo.device = context->vkDevice->Handle();
		allocatorCreateInfo.instance = context->vkInstance->Handle();
		allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

		if (vmaCreateAllocator(&allocatorCreateInfo, &m_allocator) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan allocator");

		Logger::Log<SUCCESS>("Vulkan memory allocator (Vma): ok");
	}

	VkResult Allocator::CreateBuffer(const VkBufferCreateInfo*		bufferCreateInfo,
												 const VmaAllocationCreateInfo* allocCreateInfo, VkBuffer* buffer,
												 VmaAllocation* allocation, VmaAllocationInfo* allocationInfo) const
	{
		return vmaCreateBuffer(m_allocator, bufferCreateInfo, allocCreateInfo, buffer, allocation, allocationInfo);
	}

	void Allocator::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation) const
	{
		vmaDestroyBuffer(m_allocator, buffer, allocation);
	}

	VkResult Allocator::CreateImage(const VkImageCreateInfo*	   imageCreateInfo,
												const VmaAllocationCreateInfo* allocCreateInfo, VkImage* image,
												VmaAllocation* allocation, VmaAllocationInfo* allocationInfo) const
	{
		return vmaCreateImage(m_allocator, imageCreateInfo, allocCreateInfo, image, allocation, allocationInfo);
	}

	void Allocator::DestroyImage(VkImage image, VmaAllocation allocation) const
	{
		vmaDestroyImage(m_allocator, image, allocation);
	}

	VkResult Allocator::MapMemory(VmaAllocation allocation, void** data) const
	{
		return vmaMapMemory(m_allocator, allocation, data);
	}

	void Allocator::UnmapMemory(VmaAllocation allocation) const
	{
		return vmaUnmapMemory(m_allocator, allocation);
	}

	Allocator::~Allocator()
	{
		vmaDestroyAllocator(m_allocator);
	}

	VmaAllocator Allocator::Handle() const
	{
		return m_allocator;
	}
}
