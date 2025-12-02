//
// Created by COEZARAX on 02/12/2025.
//

#include "BetterThanNothing.hpp"
#include "CVulkanBuffer_New.hpp"

namespace BetterThanNothing
{
	CVulkanBuffer_New::CVulkanBuffer_New(
		CVulkanContext*					context,
		const VkDeviceSize				size,
		const VkBufferUsageFlags		usage,
		const VmaMemoryUsage			memoryUsage,
		const VmaAllocationCreateFlags	vmaFlags
	) : m_context(context), m_size(size) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = memoryUsage;
		allocCreateInfo.flags = vmaFlags;

		if (m_context->vkMemoryAllocator->CreateBuffer(&bufferInfo, &allocCreateInfo, &m_buffer, &m_allocation, &m_allocInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan buffer.");
	}

	CVulkanBuffer_New::~CVulkanBuffer_New()
	{
		if (m_buffer && m_allocation)
			m_context->vkMemoryAllocator->DestroyBuffer(m_buffer, m_allocation);
	}

	void CVulkanBuffer_New::Write(const void* data, const VkDeviceSize size, const VkDeviceSize offset) const
	{
		if (!m_allocInfo.pMappedData)
			throw CVulkanBufferError("Cannot write to unmapped buffer");

		if (size + offset > m_size)
			throw CVulkanBufferError("Buffer overflow: Update size exceeds buffer size");

		char* destAddress = static_cast<char*>(m_allocInfo.pMappedData) + offset;

		std::memcpy(destAddress, data, size);
	}

	VkBuffer CVulkanBuffer_New::Handle() const
	{
		return m_buffer;
	}

	VkDeviceSize CVulkanBuffer_New::Size() const
	{
		return m_size;
	}
}