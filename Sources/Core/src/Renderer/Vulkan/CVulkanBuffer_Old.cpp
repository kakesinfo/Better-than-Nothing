//
// Created by acoezard on 11/25/24.
//

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanBuffer_Old::CVulkanBuffer_Old(const uint32 size, const EVulkanBufferType type, CVulkanContext* vkContext): m_context(vkContext), m_size(size)
	{
		const VkBufferUsageFlags usage = _getBufferUsageByBufferType(type);
		const VkMemoryPropertyFlags properties = _getMemoryPropertiesByBufferType(type);

		if (m_context->vkMemoryAllocator == nullptr) {
			throw std::runtime_error("L'objet CVulkanMemoryAllocator est null !");
		}
		// Check si le handle interne est valide (nécessite un getter public Handle() dans l'allocateur)
		if (m_context->vkMemoryAllocator->Handle() == VK_NULL_HANDLE) {
			throw std::runtime_error("Le handle VMA (m_allocator) est null/invalide !");
		}

		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = usage;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		const VmaAllocationCreateFlags flags = (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			? VMA_ALLOCATION_CREATE_MAPPED_BIT
			: 0;

		if (m_context->vkMemoryAllocator->CreateBuffer(&bufferCreateInfo, properties, flags, &m_buffer, &m_allocation, &m_allocationInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan buffer");
	}

	CVulkanBuffer_Old::~CVulkanBuffer_Old()
	{
		if (m_buffer != VK_NULL_HANDLE && m_allocation != VK_NULL_HANDLE)
			m_context->vkMemoryAllocator->DestroyBuffer(m_buffer, m_allocation);
	}

	CVulkanBuffer_Old::CVulkanBuffer_Old(CVulkanBuffer_Old&& other) noexcept
	{
		_move(std::move(other));
	}

	CVulkanBuffer_Old& CVulkanBuffer_Old::operator=(CVulkanBuffer_Old&& other) noexcept
	{
		_move(std::move(other));
		return *this;
	}

	VkResult CVulkanBuffer_Old::MapMemory(void** data) const
	{
		return m_context->vkMemoryAllocator->MapMemory(m_allocation, data);
	}

	void CVulkanBuffer_Old::UnmapMemory() const
	{
		return m_context->vkMemoryAllocator->UnmapMemory(m_allocation);
	}

	void* CVulkanBuffer_Old::GetMappedData() const
	{
		return m_allocationInfo.pMappedData;
	}

	void CVulkanBuffer_Old::Write(const void* data, const size_t size, const size_t offset) const
	{
		if (!m_allocationInfo.pMappedData)
			throw CVulkanBufferError("Cannot write to unmapped buffer");

		if (size + offset > m_size)
			throw CVulkanBufferError("Buffer overflow: Update size exceeds buffer size");

		char* destAddress = static_cast<char*>(m_allocationInfo.pMappedData) + offset;

		std::memcpy(destAddress, data, size);
	}

	uint32 CVulkanBuffer_Old::Size() const
	{
		return m_size;
	}

	VkBuffer CVulkanBuffer_Old::Handle() const
	{
		return m_buffer;
	}

	void CVulkanBuffer_Old::_move(CVulkanBuffer_Old&& other) noexcept
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

	VkBufferUsageFlags CVulkanBuffer_Old::_getBufferUsageByBufferType(const EVulkanBufferType type)
	{
		switch (type)
		{
			case EVulkanBufferType::STAGING:
				return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			case EVulkanBufferType::VERTEX:
				return VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			case EVulkanBufferType::INDEX:
				return VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			case EVulkanBufferType::UNIFORM:
				return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			default:
				throw std::runtime_error("Invalid Vulkan buffer type");
		}
	}

	VkMemoryPropertyFlags CVulkanBuffer_Old::_getMemoryPropertiesByBufferType(const EVulkanBufferType type)
	{
		switch (type)
		{
			case EVulkanBufferType::STAGING:
			case EVulkanBufferType::UNIFORM:
				return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			case EVulkanBufferType::VERTEX:
			case EVulkanBufferType::INDEX:
				return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			default:
				throw std::runtime_error("Invalid Vulkan buffer type");
		}
	}
}