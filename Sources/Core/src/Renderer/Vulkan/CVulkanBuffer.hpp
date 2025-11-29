//
// Created by acoezard on 11/25/24.
//

#pragma once

#include "CVulkanContext.hpp"
#include "EVulkanBufferType.hpp"

namespace BetterThanNothing
{
	class CVulkanBuffer
	{
	private:
		CVulkanContext* m_context = nullptr;

		uint32 m_size = 0;

		VkBuffer m_buffer = VK_NULL_HANDLE;

		VmaAllocation m_allocation = VK_NULL_HANDLE;
		VmaAllocationInfo m_allocationInfo = {};

	public:
		CVulkanBuffer() = default;
		CVulkanBuffer(uint32 size, EVulkanBufferType type, CVulkanContext* context);
		~CVulkanBuffer();

		CVulkanBuffer(const CVulkanInstance&) = delete;
		CVulkanBuffer& operator=(const CVulkanBuffer&) = delete;

		CVulkanBuffer(CVulkanBuffer&&) noexcept;
		CVulkanBuffer& operator=(CVulkanBuffer&&) noexcept;

		VkResult MapMemory(void** data) const;
		void UnmapMemory() const;

		[[nodiscard]]
		uint32 Size() const;

		[[nodiscard]]
		VkBuffer Handle() const;

	private:
		void _move(CVulkanBuffer&& other) noexcept;

		static VkBufferUsageFlags _getBufferUsageByBufferType(EVulkanBufferType type);
		static VkMemoryPropertyFlags _getMemoryPropertiesByBufferType(EVulkanBufferType type);
	};
}
