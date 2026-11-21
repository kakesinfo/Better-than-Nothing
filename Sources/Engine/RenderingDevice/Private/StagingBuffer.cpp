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
#include <algorithm>
#include <cstring>

namespace BetterThanNothing
{
	StagingBuffer::StagingBuffer(RenderingApi::Allocator* allocator, const uint64_t size) : m_allocator(allocator)
	{
		AllocateBuffer(size * 1024 * 1024);
	}

	StagingBuffer::~StagingBuffer()
	{
		if (m_allocator && m_buffer != VK_NULL_HANDLE)
			m_allocator->DestroyBuffer(m_buffer, m_bufferAlloc);
	}

	void StagingBuffer::AllocateBuffer(const uint64_t sizeBytes)
	{
		if (m_allocator && m_buffer != VK_NULL_HANDLE)
		{
			m_allocator->DestroyBuffer(m_buffer, m_bufferAlloc);
			m_buffer = VK_NULL_HANDLE;
			m_bufferAlloc = VK_NULL_HANDLE;
			m_bufferAllocInfo = {};
		}

		m_size = sizeBytes;

		VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		bufferInfo.size = m_size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

		if (m_allocator->CreateBuffer(&bufferInfo, &allocInfo, &m_buffer, &m_bufferAlloc, &m_bufferAllocInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to create staging buffer");

		if (!m_bufferAllocInfo.pMappedData)
			throw std::runtime_error("Failed to map staging buffer");
	}

	std::unique_ptr<StagingBuffer> StagingBuffer::Create(RenderingApi::Allocator* allocator, uint64_t size)
	{
		return std::make_unique<StagingBuffer>(allocator, size);
	}

	uint64_t StagingBuffer::Stage(const void* data, const uint64_t dataSize, const uint64_t alignment)
	{
		const uint64_t alignMask = alignment > 1 ? (alignment - 1) : 0;
		uint64_t	   alignedOffset = (m_offset + alignMask) & ~alignMask;

		if (dataSize + alignedOffset > m_size)
		{
			if (m_offset == 0)
			{
				const uint64_t newSize = std::max(m_size * 2, dataSize + alignMask + 1);
				Logger::Log<WARNING>("StagingBuffer resized from {} bytes to {} bytes", m_size, newSize);
				AllocateBuffer(newSize);
				alignedOffset = 0;
			}
			else
			{
				throw std::runtime_error("Staging buffer is too small");
			}
		}

		auto* destination = static_cast<uint8_t*>(m_bufferAllocInfo.pMappedData) + alignedOffset;

		std::memcpy(destination, data, dataSize);

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
