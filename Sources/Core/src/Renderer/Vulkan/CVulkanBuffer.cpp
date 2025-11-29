//
// Created by acoezard on 11/25/24.
//

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanBuffer::CVulkanBuffer(const uint32 size, const EVulkanBufferType type, CVulkanContext* vkContext): m_context(vkContext), m_size(size)
	{
		const VkBufferUsageFlags usage = _getBufferUsageByBufferType(type);
		const VkMemoryPropertyFlags properties = _getMemoryPropertiesByBufferType(type);

		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = usage;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (m_context->vkMemoryAllocator->CreateBuffer(&bufferCreateInfo, properties, &m_buffer, &m_allocation, &m_allocationInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan buffer");
	}

	CVulkanBuffer::~CVulkanBuffer()
	{
		if (m_buffer != VK_NULL_HANDLE && m_allocation != VK_NULL_HANDLE)
			m_context->vkMemoryAllocator->DestroyBuffer(m_buffer, m_allocation);
	}

	CVulkanBuffer::CVulkanBuffer(CVulkanBuffer&& other) noexcept
	{
		_move(std::move(other));
	}

	CVulkanBuffer& CVulkanBuffer::operator=(CVulkanBuffer&& other) noexcept
	{
		_move(std::move(other));
		return *this;
	}

	VkResult CVulkanBuffer::MapMemory(void** data) const
	{
		return m_context->vkMemoryAllocator->MapMemory(m_allocation, data);
	}

	void CVulkanBuffer::UnmapMemory() const
	{
		return m_context->vkMemoryAllocator->UnmapMemory(m_allocation);
	}

	uint32 CVulkanBuffer::Size() const
	{
		return m_size;
	}

	VkBuffer CVulkanBuffer::Handle() const
	{
		return m_buffer;
	}

	void CVulkanBuffer::_move(CVulkanBuffer&& other) noexcept
	{
		m_context = other.m_context;
		m_size = other.m_size;
		m_buffer = other.m_buffer;
		m_allocation = other.m_allocation;
		m_allocationInfo = other.m_allocationInfo;

		other.m_context = nullptr;
		other.m_size = 0;
		other.m_buffer = VK_NULL_HANDLE;
		other.m_allocation = VK_NULL_HANDLE;
		other.m_allocationInfo = {};
	}

	VkBufferUsageFlags CVulkanBuffer::_getBufferUsageByBufferType(const EVulkanBufferType type)
	{
		switch (type)
		{
			case EVulkanBufferType::STAGING:
				return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			case EVulkanBufferType::VERTEX:
				return VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			case EVulkanBufferType::INDEX:
				return VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			default:
				throw std::runtime_error("Invalid Vulkan buffer type");
		}
	}

	VkMemoryPropertyFlags CVulkanBuffer::_getMemoryPropertiesByBufferType(const EVulkanBufferType type)
	{
		switch (type)
		{
			case EVulkanBufferType::STAGING:
				return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			case EVulkanBufferType::VERTEX:
			case EVulkanBufferType::INDEX:
				return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			default:
				throw std::runtime_error("Invalid Vulkan buffer type");
		}
	}
}