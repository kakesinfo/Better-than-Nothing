//
// Created by COEZARAX on 02/12/2025.
//

#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class CVulkanBufferError final : public std::runtime_error
	{
	public:
		explicit CVulkanBufferError(const std::string& message)
			: std::runtime_error(message) {}
	};

	class CVulkanBuffer
	{
	public:
		CVulkanBuffer(CVulkanContext* context, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags vmaFlags);
		virtual ~CVulkanBuffer();

		CVulkanBuffer(const CVulkanBuffer&) = delete;
		CVulkanBuffer& operator=(const CVulkanBuffer&) = delete;

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
