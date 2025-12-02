//
// Created by acoezard on 11/25/24.
//

#pragma once

#include "CVulkanContext.hpp"
#include "EVulkanBufferType.hpp"

namespace BetterThanNothing
{
	class CVulkanBufferError final : public std::runtime_error
	{
	public:
		explicit CVulkanBufferError(const std::string& message)
			: std::runtime_error(message) {}
	};

	class CVulkanBuffer_Old
	{
	private:
		CVulkanContext* m_context = nullptr;

		uint32 m_size = 0;

		VkBuffer m_buffer = VK_NULL_HANDLE;

		VmaAllocation m_allocation = VK_NULL_HANDLE;
		VmaAllocationInfo m_allocationInfo = {};

	public:
		CVulkanBuffer_Old() = default;
		CVulkanBuffer_Old(uint32 size, EVulkanBufferType type, CVulkanContext* context);
		~CVulkanBuffer_Old();

		CVulkanBuffer_Old(const CVulkanInstance&) = delete;
		CVulkanBuffer_Old& operator=(const CVulkanBuffer_Old&) = delete;

		CVulkanBuffer_Old(CVulkanBuffer_Old&&) noexcept;
		CVulkanBuffer_Old& operator=(CVulkanBuffer_Old&&) noexcept;

		VkResult MapMemory(void** data) const;
		void UnmapMemory() const;
		void* GetMappedData() const;

		template <typename T>
		void Write(const T& data, const size_t offset = 0) const
		{
			Write(&data, sizeof(T), offset);
		}

		void Write(const void* data, const size_t size, const size_t offset = 0) const;

		uint32 Size() const;

		[[nodiscard]]
		VkBuffer Handle() const;

	private:
		void _move(CVulkanBuffer_Old&& other) noexcept;

		static VkBufferUsageFlags _getBufferUsageByBufferType(EVulkanBufferType type);
		static VkMemoryPropertyFlags _getMemoryPropertiesByBufferType(EVulkanBufferType type);
	};
}
