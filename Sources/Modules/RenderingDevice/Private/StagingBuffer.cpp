/*
*	Better than Nothing
 *	Copyright (C) 2026 kakesinfo
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

#include "StagingBuffer.hpp"

#include "Common/Logger.hpp"
#include "RenderingApi/Core/Allocator.hpp"

namespace BetterThanNothing
{
	StagingBuffer::StagingBuffer(RenderingApi::Allocator* allocator, const uint64_t size): m_allocator(allocator)
	{
		m_size = size * 1024 * 1024;

		VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		bufferInfo.size = m_size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		if (m_allocator->CreateBuffer(&bufferInfo, &allocInfo, &m_buffer, &m_bufferAlloc, &m_bufferAllocInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to create staging buffer");

		if (!m_bufferAllocInfo.pMappedData)
			throw std::runtime_error("Failed to map staging buffer");
	}

	StagingBuffer::~StagingBuffer()
	{
		if (m_allocator && m_buffer)
			m_allocator->DestroyBuffer(m_buffer, m_bufferAlloc);
	}

	std::unique_ptr<StagingBuffer> StagingBuffer::Create(RenderingApi::Allocator* allocator, uint64_t size)
	{
		return std::make_unique<StagingBuffer>(allocator, size);
	}

	uint64_t StagingBuffer::Stage(const void* data, const uint64_t dataSize, const uint32_t alignment)
	{
		const uint32_t alignedOffset = (m_offset + alignment - 1) & ~(alignment - 1);

		if (dataSize + alignedOffset >= m_size)
			throw std::runtime_error("Staging buffer is too small");

		auto* destination = static_cast<uint8_t*>(m_bufferAllocInfo.pMappedData) + alignedOffset;

		memcpy(destination, data, dataSize);

		m_offset = alignedOffset + dataSize;

		return alignedOffset;
	}

	VkBuffer StagingBuffer::GetBuffer() const
	{
		return m_buffer;
	}

	void StagingBuffer::Reset()
	{
		m_offset = 0;
	}
}
