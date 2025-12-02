//
// Created by COEZARAX on 02/12/2025.
//

#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class CVulkanBuffer_New
	{
	public:
		CVulkanBuffer_New(CVulkanContext* context, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags vmaFlags);
		virtual ~CVulkanBuffer_New();

		CVulkanBuffer_New(const CVulkanBuffer_New&) = delete;
		CVulkanBuffer_New& operator=(const CVulkanBuffer_New&) = delete;

		[[nodiscard]] VkBuffer Handle() const;
		[[nodiscard]] VkDeviceSize Size() const;

		void Write(const void* data, VkDeviceSize size, VkDeviceSize offset = 0) const;

	protected:
		CVulkanContext*		m_context;
		VkBuffer			m_buffer = VK_NULL_HANDLE;
		VmaAllocation		m_allocation = VK_NULL_HANDLE;
		VmaAllocationInfo	m_allocInfo = {};
		VkDeviceSize		m_size;
	};
}
