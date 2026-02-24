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

#include "RenderingDevice/BufferRegistry.hpp"

#include "RenderingApi/Core/Allocator.hpp"
#include "RenderingApi/Core/CommandBuffer.hpp"
#include "RenderingApi/Core/Device.hpp"
#include "RenderingApi/Resources/Buffer.hpp"

#include <cstring>

namespace BetterThanNothing
{
	BufferRegistry::BufferRegistry(const BufferRegistryCreateInfo& createInfo) : m_createInfo(createInfo) {}

	BufferRegistry::~BufferRegistry()
	{
		for (std::uint32_t i = 0; i < m_buffers.Size(); i++)
		{
			const auto buffer = m_buffers.Get(i);
			m_createInfo.allocator->DestroyBuffer(buffer->value, buffer->allocation);
			m_buffers.Destroy(i);
		}
	}

	std::unique_ptr<BufferRegistry> BufferRegistry::Create(const BufferRegistryCreateInfo& createInfo)
	{
		return std::make_unique<BufferRegistry>(createInfo);
	}

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

	static bool Internal_CreateBuffer(const BufferDescription& desc, RenderingApi::Buffer* buffer, RenderingApi::Allocator* allocator)
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

		if (allocator->CreateBuffer(&bufferInfo, &allocInfo, &buffer->value, &buffer->allocation, &buffer->allocInfo) != VK_SUCCESS)
			return false;

		buffer->size = desc.size;

		return true;
	}

	Handle<RenderingApi::Buffer> BufferRegistry::CreateBuffer(const BufferDescription& desc)
	{
		const Handle<RenderingApi::Buffer> handle = m_buffers.Allocate();

		if (!Internal_CreateBuffer(desc, m_buffers.Get(handle), m_createInfo.allocator))
		{
			m_buffers.Destroy(handle);
			throw std::runtime_error("Failed to vulkan create buffer!");
		}

		return handle;
	}

	Handle<RenderingApi::Buffer> BufferRegistry::CreateVertexBuffer(const void* data, size_t size)
	{
		BufferDescription desc{};
		desc.size = size;
		desc.bufferUsage = BufferUsageFlags::TransferDst | BufferUsageFlags::Vertex;
		desc.memoryUsage = BufferMemoryUsage::GpuOnly;

		const Handle<RenderingApi::Buffer> handle = CreateBuffer(desc);

		if (!CopyDataToBuffer(handle, data, size))
			throw std::runtime_error("Failed to copy data to buffer!");

		return handle;
	}

	Handle<RenderingApi::Buffer> BufferRegistry::CreateIndexBuffer(const void* data, size_t size)
	{
		BufferDescription desc{};
		desc.size = size;
		desc.bufferUsage = BufferUsageFlags::TransferDst | BufferUsageFlags::Index;
		desc.memoryUsage = BufferMemoryUsage::GpuOnly;

		const Handle<RenderingApi::Buffer> handle = CreateBuffer(desc);

		if (!CopyDataToBuffer(handle, data, size))
			throw std::runtime_error("Failed to copy data to buffer!");

		return handle;
	}

	bool BufferRegistry::CopyDataToBuffer(Handle<RenderingApi::Buffer> dstBufferHandle, const void* data, size_t size)
	{
		if (data == nullptr || size == 0)
			return false;

		VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		RenderingApi::Buffer stagingBuffer; // TODO: replace this with a persistent 128B shared buffer

		if (m_createInfo.allocator->CreateBuffer(&bufferInfo, &allocInfo, &stagingBuffer.value, &stagingBuffer.allocation, &stagingBuffer.allocInfo) != VK_SUCCESS)
			return false;

		if (!stagingBuffer.allocInfo.pMappedData)
			return false;

		std::memcpy(stagingBuffer.allocInfo.pMappedData, data, size);

		RenderingApi::CommandBuffer::SingleTimeCommands(
			[&](const std::unique_ptr<RenderingApi::CommandBuffer>& cmd) {
				const VkBuffer dstBuffer = m_buffers.Get(dstBufferHandle)->value;
				cmd->CopyBuffer(stagingBuffer.value, dstBuffer, size);
			},
			m_createInfo.device, m_createInfo.commandPool, m_createInfo.graphicsQueue
		);

		m_createInfo.allocator->DestroyBuffer(stagingBuffer.value, stagingBuffer.allocation);

		return true;
	}

	void BufferRegistry::DestroyBuffer(Handle<RenderingApi::Buffer> handle)
	{
		if (!m_buffers.Has(handle))
			throw std::runtime_error("Invalid buffer handle!");

		const RenderingApi::Buffer* buffer = m_buffers.Get(handle);
		m_createInfo.allocator->DestroyBuffer(buffer->value, buffer->allocation);
		m_buffers.Destroy(handle);
	}

	RenderingApi::Buffer* BufferRegistry::GetBuffer(Handle<RenderingApi::Buffer> handle)
	{
		if (!m_buffers.Has(handle))
			throw std::runtime_error("Invalid buffer handle!");
		return m_buffers.Get(handle);
	}
}
