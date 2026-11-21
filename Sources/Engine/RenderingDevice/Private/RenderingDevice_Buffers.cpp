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

#include "RenderingDevice/RenderingDevice.hpp"
#include "RenderingDevice_Internals.hpp"

#include "RenderingApi/Resources/Buffer.hpp"
#include "RenderingDevice/BufferDescription.hpp"
#include "RenderingDevice/Handle.hpp"
#include "StagingBuffer.hpp"

#include <Common/Logger.hpp>
#include <RenderingApi.hpp>
#include <cmath>
#include <cstring>

namespace BetterThanNothing
{
	static VmaMemoryUsage Internal_RetrieveMemoryUsage(const BufferMemoryUsage& memoryUsage)
	{
		switch (memoryUsage)
		{
			case BufferMemoryUsage::Auto:
				return VMA_MEMORY_USAGE_AUTO;
			case BufferMemoryUsage::CpuCopy:
				return VMA_MEMORY_USAGE_CPU_COPY;
			case BufferMemoryUsage::CpuOnly:
				return VMA_MEMORY_USAGE_CPU_ONLY;
			case BufferMemoryUsage::GpuOnly:
				return VMA_MEMORY_USAGE_GPU_ONLY;
			case BufferMemoryUsage::CpuToGpu:
				return VMA_MEMORY_USAGE_CPU_TO_GPU;
			case BufferMemoryUsage::GpuToCpu:
				return VMA_MEMORY_USAGE_GPU_TO_CPU;
			case BufferMemoryUsage::None:
			default:
				throw std::runtime_error("Invalid memory usage");
		}
	}

	static bool Internal_CreateBuffer(const BufferDescription& desc, RenderingApi::Buffer* buffer, const RenderingApi::Allocator* allocator)
	{
		VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		bufferInfo.size = desc.size;
		bufferInfo.usage = static_cast<VkBufferUsageFlags>(desc.bufferUsage);

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = Internal_RetrieveMemoryUsage(desc.memoryUsage);

		if (desc.allocationFlags & BufferAllocationFlags::Mapped)
			allocInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
		if (desc.allocationFlags & BufferAllocationFlags::Dedicated)
			allocInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		if (desc.allocationFlags & BufferAllocationFlags::HostAccessSequentialWrite)
			allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

		if (allocator->CreateBuffer(&bufferInfo, &allocInfo, &buffer->value, &buffer->allocation, &buffer->allocInfo) != VK_SUCCESS)
			return false;

		buffer->size = desc.size;

		return true;
	}

	Handle RenderingDevice::CreateBuffer(const BufferDescription& desc)
	{
		const Handle handle = m_self->buffers.Allocate();

		if (!Internal_CreateBuffer(desc, m_self->buffers.Get(handle), m_self->allocator.get()))
		{
			m_self->buffers.Destroy(handle);
			throw std::runtime_error("Failed to create Vulkan buffer!");
		}

		return handle;
	}

	Handle RenderingDevice::CreateUniformBuffer(const size_t size)
	{
		BufferDescription desc{};
		desc.size = size;
		desc.bufferUsage = BufferUsageFlags::Uniform;
		desc.memoryUsage = BufferMemoryUsage::Auto;
		desc.allocationFlags = BufferAllocationFlags::HostAccessSequentialWrite | BufferAllocationFlags::Mapped;

		return CreateBuffer(desc);
	}

	Handle RenderingDevice::CreateStorageBuffer(const size_t size)
	{
		BufferDescription desc{};
		desc.size = size;
		desc.bufferUsage = BufferUsageFlags::Storage;
		desc.memoryUsage = BufferMemoryUsage::Auto;
		desc.allocationFlags = BufferAllocationFlags::HostAccessSequentialWrite | BufferAllocationFlags::Mapped;

		return CreateBuffer(desc);
	}

	Handle RenderingDevice::CreateVertexBuffer(const void* data, const size_t size)
	{
		BufferDescription desc{};
		desc.size = size;
		desc.bufferUsage = BufferUsageFlags::TransferDst | BufferUsageFlags::Vertex;
		desc.memoryUsage = BufferMemoryUsage::GpuOnly;

		const Handle handle = CreateBuffer(desc);

		if (!StageDataToBuffer(handle, data, size))
		{
			DestroyBuffer(handle);
			throw std::runtime_error("Failed to copy data to vertex buffer!");
		}

		return handle;
	}

	Handle RenderingDevice::CreateIndexBuffer(const void* data, const size_t size)
	{
		BufferDescription desc{};
		desc.size = size;
		desc.bufferUsage = BufferUsageFlags::TransferDst | BufferUsageFlags::Index;
		desc.memoryUsage = BufferMemoryUsage::GpuOnly;

		const Handle handle = CreateBuffer(desc);

		if (!StageDataToBuffer(handle, data, size))
		{
			DestroyBuffer(handle);
			throw std::runtime_error("Failed to copy data to index buffer!");
		}

		return handle;
	}

	bool RenderingDevice::StageDataToBuffer(const Handle& dstBufferHandle, const void* data, const size_t size)
	{
		if (data == nullptr || size == 0)
			return false;

		Internal_EnsureUploadBuffer(size, 4);

		const uint64_t offset = m_self->stagingBuffer->Stage(data, size, 4);
		const VkBuffer srcBuffer = m_self->stagingBuffer->GetBuffer();
		const VkBuffer dstBuffer = m_self->buffers.Get(dstBufferHandle)->value;

		m_self->uploadCommandBuffer->CopyBuffer(srcBuffer, dstBuffer, size, offset);

		if (m_self->uploadBatchDepth == 0)
			Internal_FlushUploadBuffer();

		return true;
	}

	bool RenderingDevice::CopyDataToBuffer(const Handle& dstBufferHandle, const void* data, const size_t size)
	{
		if (data == nullptr || size == 0)
			return false;

		const RenderingApi::Buffer* buffer = m_self->buffers.Get(dstBufferHandle);
		if (buffer->allocInfo.pMappedData == nullptr || size > buffer->size)
			return false;

		std::memcpy(buffer->allocInfo.pMappedData, data, size);

		return true;
	}

	void RenderingDevice::DestroyBuffer(const Handle& handle)
	{
		if (!m_self->buffers.Has(handle))
			throw std::runtime_error("Invalid buffer handle!");

		const RenderingApi::Buffer* buffer = m_self->buffers.Get(handle);
		const VkBuffer				vkBuffer = buffer->value;
		const VmaAllocation			allocation = buffer->allocation;
		m_self->buffers.Destroy(handle);

		RenderingApi::Allocator* allocator = m_self->allocator.get();
		Internal_EnqueueDeletion([allocator, vkBuffer, allocation]() {
			allocator->DestroyBuffer(vkBuffer, allocation);
		});
	}

	RenderingApi::Buffer* RenderingDevice::GetBuffer(const Handle& handle)
	{
		if (!m_self->buffers.Has(handle))
			throw std::runtime_error("Invalid buffer handle!");
		return m_self->buffers.Get(handle);
	}

	void* RenderingDevice::GetBufferData(const Handle& handle)
	{
		if (!m_self->buffers.Has(handle))
			throw std::runtime_error("Invalid buffer handle!");

		return m_self->buffers.Get(handle)->allocInfo.pMappedData;
	}
}
