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

#include "RenderingDevice/BufferDescription.hpp"
#include "RenderingDevice/Handle.hpp"
#include "RenderingDevice/RenderingDevice.hpp"
#include "RenderingDevice_Internals.hpp"

#include <Common/Logger.hpp>
#include <RenderingApi.hpp>
#include <cstring>

namespace BetterThanNothing
{
	static VkDeviceAddress AlignUpAddress(const VkDeviceAddress addr, const VkDeviceSize alignment)
	{
		if (alignment <= 1)
			return addr;
		return (addr + alignment - 1) & ~(alignment - 1);
	}

	bool RenderingDevice::IsRayQuerySupported() const
	{
		return m_self->device->IsRayQuerySupported();
	}

	Handle RenderingDevice::CreateBlas(const Handle& vertexBuffer, const uint32_t vertexCount, const uint32_t vertexStride, const Handle& indexBuffer, const uint32_t indexCount)
	{
		if (!IsRayQuerySupported() || vertexCount == 0 || indexCount < 3)
			return {};

		if (m_self->uploadRecording)
			Internal_FlushUploadBuffer();

		const VkDeviceAddress vertexAddress = GetBufferDeviceAddress(vertexBuffer);
		const VkDeviceAddress indexAddress = GetBufferDeviceAddress(indexBuffer);
		const uint32_t		  primitiveCount = indexCount / 3;

		VkAccelerationStructureGeometryTrianglesDataKHR triangles{};
		triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		triangles.vertexData.deviceAddress = vertexAddress;
		triangles.vertexStride = vertexStride;
		triangles.maxVertex = vertexCount - 1;
		triangles.indexType = VK_INDEX_TYPE_UINT32;
		triangles.indexData.deviceAddress = indexAddress;

		VkAccelerationStructureGeometryKHR geometry{};
		geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		geometry.geometry.triangles = triangles;

		VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
		buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.geometryCount = 1;
		buildInfo.pGeometries = &geometry;

		VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
		sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		m_self->device->GetAccelerationStructureBuildSizes(VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &primitiveCount, &sizeInfo);

		const Handle						 asHandle = m_self->accelerationStructures.Allocate();
		RenderingApi::AccelerationStructure* as = m_self->accelerationStructures.Get(asHandle);

		VkBufferCreateInfo asBufInfo{};
		asBufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		asBufInfo.size = sizeInfo.accelerationStructureSize;
		asBufInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

		VmaAllocationCreateInfo asAllocInfo{};
		asAllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

		if (m_self->allocator->CreateBuffer(&asBufInfo, &asAllocInfo, &as->buffer, &as->allocation, nullptr) != VK_SUCCESS)
		{
			m_self->accelerationStructures.Destroy(asHandle);
			throw std::runtime_error("Failed to create BLAS storage buffer");
		}
		as->size = sizeInfo.accelerationStructureSize;

		VkAccelerationStructureCreateInfoKHR asCreateInfo{};
		asCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		asCreateInfo.buffer = as->buffer;
		asCreateInfo.size = sizeInfo.accelerationStructureSize;
		asCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

		if (m_self->device->CreateAccelerationStructure(&asCreateInfo, &as->handle) != VK_SUCCESS)
		{
			m_self->allocator->DestroyBuffer(as->buffer, as->allocation);
			m_self->accelerationStructures.Destroy(asHandle);
			throw std::runtime_error("Failed to create BLAS");
		}

		// Temporary scratch buffer with proper alignment padding
		const uint32_t scratchAlign = m_self->device->GetMinAccelerationStructureScratchOffsetAlignment();
		VkBufferCreateInfo scratchBufInfo{};
		scratchBufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		scratchBufInfo.size = sizeInfo.buildScratchSize + scratchAlign;
		scratchBufInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

		VkBuffer	  scratchBuffer = VK_NULL_HANDLE;
		VmaAllocation scratchAllocation = VK_NULL_HANDLE;
		if (m_self->allocator->CreateBuffer(&scratchBufInfo, &asAllocInfo, &scratchBuffer, &scratchAllocation, nullptr) != VK_SUCCESS)
		{
			DestroyAccelerationStructure(asHandle);
			throw std::runtime_error("Failed to create BLAS scratch buffer");
		}

		buildInfo.dstAccelerationStructure = as->handle;
		buildInfo.scratchData.deviceAddress = AlignUpAddress(m_self->device->GetBufferDeviceAddress(scratchBuffer), scratchAlign);

		VkAccelerationStructureBuildRangeInfoKHR rangeInfo{};
		rangeInfo.primitiveCount = primitiveCount;
		rangeInfo.primitiveOffset = 0;
		rangeInfo.firstVertex = 0;
		rangeInfo.transformOffset = 0;
		const VkAccelerationStructureBuildRangeInfoKHR* pRangeInfo = &rangeInfo;

		RenderingApi::CommandBuffer::SingleTimeCommands(
			[&](std::unique_ptr<RenderingApi::CommandBuffer>& cmd) {
				m_self->device->CmdBuildAccelerationStructures(cmd->Handle(), 1, &buildInfo, &pRangeInfo);
				cmd->MemoryBarrier2(
					VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
					VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
					VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
					VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR
				);
			},
			m_self->device.get(),
			m_self->commandPool.get(),
			m_self->graphicsQueue.get()
		);

		m_self->allocator->DestroyBuffer(scratchBuffer, scratchAllocation);

		VkAccelerationStructureDeviceAddressInfoKHR addrInfo{};
		addrInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		addrInfo.accelerationStructure = as->handle;
		as->deviceAddress = m_self->device->GetAccelerationStructureDeviceAddress(&addrInfo);

		return asHandle;
	}

	void RenderingDevice::BuildOrUpdateTlas(Handle& tlasHandle, Handle& scratchBufferHandle, Handle& instanceBufferHandle, const std::span<const TlasInstanceDesc> instances)
	{
		if (!IsRayQuerySupported())
			return;

		const uint32_t instanceCount = static_cast<uint32_t>(instances.size());
		const size_t   requiredInstanceBytes = std::max<size_t>(1, instanceCount) * sizeof(VkAccelerationStructureInstanceKHR);

		if (!instanceBufferHandle.IsValid() || GetBuffer(instanceBufferHandle)->size < requiredInstanceBytes)
		{
			if (instanceBufferHandle.IsValid())
				DestroyBuffer(instanceBufferHandle);

			BufferDescription desc{};
			desc.size = requiredInstanceBytes * 2;
			desc.bufferUsage = BufferUsageFlags::AccelerationStructureBuildInputReadOnly | BufferUsageFlags::ShaderDeviceAddress;
			desc.memoryUsage = BufferMemoryUsage::Auto;
			desc.allocationFlags = BufferAllocationFlags::HostAccessSequentialWrite | BufferAllocationFlags::Mapped;
			instanceBufferHandle = CreateBuffer(desc);
		}

		auto* mappedInstances = static_cast<VkAccelerationStructureInstanceKHR*>(GetBufferData(instanceBufferHandle));
		for (uint32_t i = 0; i < instanceCount; ++i)
		{
			const auto& src = instances[i];
			const auto* blas = m_self->accelerationStructures.Get(src.blasHandle);

			VkAccelerationStructureInstanceKHR dst{};
			std::memcpy(&dst.transform, src.transform, sizeof(VkTransformMatrixKHR));
			dst.instanceCustomIndex = src.instanceCustomIndex & 0xFFFFFFu;
			dst.mask = src.mask & 0xFFu;
			dst.instanceShaderBindingTableRecordOffset = 0;
			dst.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
			dst.accelerationStructureReference = blas->deviceAddress;
			mappedInstances[i] = dst;
		}

		VkAccelerationStructureGeometryInstancesDataKHR instancesData{};
		instancesData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		instancesData.arrayOfPointers = VK_FALSE;
		instancesData.data.deviceAddress = GetBufferDeviceAddress(instanceBufferHandle);

		VkAccelerationStructureGeometryKHR geometry{};
		geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		geometry.geometry.instances = instancesData;

		VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
		buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.geometryCount = 1;
		buildInfo.pGeometries = &geometry;

		VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
		sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		m_self->device->GetAccelerationStructureBuildSizes(VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &instanceCount, &sizeInfo);

		const bool needsNewTlas = !tlasHandle.IsValid() || m_self->accelerationStructures.Get(tlasHandle)->size < sizeInfo.accelerationStructureSize;
		if (needsNewTlas)
		{
			if (tlasHandle.IsValid())
				DestroyAccelerationStructure(tlasHandle);

			tlasHandle = m_self->accelerationStructures.Allocate();
			RenderingApi::AccelerationStructure* tlas = m_self->accelerationStructures.Get(tlasHandle);

			const VkDeviceSize allocSize = std::max<VkDeviceSize>(sizeInfo.accelerationStructureSize, 65536);

			VkBufferCreateInfo asBufInfo{};
			asBufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			asBufInfo.size = allocSize;
			asBufInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

			VmaAllocationCreateInfo asAllocInfo{};
			asAllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

			if (m_self->allocator->CreateBuffer(&asBufInfo, &asAllocInfo, &tlas->buffer, &tlas->allocation, nullptr) != VK_SUCCESS)
			{
				m_self->accelerationStructures.Destroy(tlasHandle);
				tlasHandle = {};
				throw std::runtime_error("Failed to create TLAS storage buffer");
			}
			tlas->size = allocSize;

			VkAccelerationStructureCreateInfoKHR asCreateInfo{};
			asCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
			asCreateInfo.buffer = tlas->buffer;
			asCreateInfo.size = allocSize;
			asCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

			if (m_self->device->CreateAccelerationStructure(&asCreateInfo, &tlas->handle) != VK_SUCCESS)
			{
				m_self->allocator->DestroyBuffer(tlas->buffer, tlas->allocation);
				m_self->accelerationStructures.Destroy(tlasHandle);
				tlasHandle = {};
				throw std::runtime_error("Failed to create TLAS");
			}

			VkAccelerationStructureDeviceAddressInfoKHR addrInfo{};
			addrInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
			addrInfo.accelerationStructure = tlas->handle;
			tlas->deviceAddress = m_self->device->GetAccelerationStructureDeviceAddress(&addrInfo);
		}

		const uint32_t scratchAlign = m_self->device->GetMinAccelerationStructureScratchOffsetAlignment();
		const size_t   requiredScratchBytes = sizeInfo.buildScratchSize + scratchAlign;
		if (!scratchBufferHandle.IsValid() || GetBuffer(scratchBufferHandle)->size < requiredScratchBytes)
		{
			if (scratchBufferHandle.IsValid())
				DestroyBuffer(scratchBufferHandle);

			BufferDescription desc{};
			desc.size = std::max<size_t>(requiredScratchBytes * 2, 65536);
			desc.bufferUsage = BufferUsageFlags::Storage | BufferUsageFlags::ShaderDeviceAddress;
			desc.memoryUsage = BufferMemoryUsage::GpuOnly;
			scratchBufferHandle = CreateBuffer(desc);
		}

		RenderingApi::AccelerationStructure* tlas = m_self->accelerationStructures.Get(tlasHandle);
		buildInfo.dstAccelerationStructure = tlas->handle;
		buildInfo.scratchData.deviceAddress = AlignUpAddress(GetBufferDeviceAddress(scratchBufferHandle), scratchAlign);

		VkAccelerationStructureBuildRangeInfoKHR rangeInfo{};
		rangeInfo.primitiveCount = instanceCount;
		rangeInfo.primitiveOffset = 0;
		rangeInfo.firstVertex = 0;
		rangeInfo.transformOffset = 0;
		const VkAccelerationStructureBuildRangeInfoKHR* pRangeInfo = &rangeInfo;

		const auto cmd = m_self->commandBuffers[m_currentFrameIndex].get();
		m_self->device->CmdBuildAccelerationStructures(cmd->Handle(), 1, &buildInfo, &pRangeInfo);
		cmd->MemoryBarrier2(
			VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
			VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
			VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
			VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR
		);
	}

	void RenderingDevice::DestroyAccelerationStructure(const Handle& handle)
	{
		if (!m_self->accelerationStructures.Has(handle))
			throw std::runtime_error("Invalid acceleration structure handle!");

		const auto*				   as = m_self->accelerationStructures.Get(handle);
		const VkAccelerationStructureKHR vkAs = as->handle;
		const VkBuffer					 vkBuffer = as->buffer;
		const VmaAllocation				 allocation = as->allocation;
		m_self->accelerationStructures.Destroy(handle);

		RenderingApi::Device*	 device = m_self->device.get();
		RenderingApi::Allocator* allocator = m_self->allocator.get();
		Internal_EnqueueDeletion([device, allocator, vkAs, vkBuffer, allocation]() {
			if (vkAs != VK_NULL_HANDLE)
				device->DestroyAccelerationStructure(vkAs);
			if (vkBuffer != VK_NULL_HANDLE)
				allocator->DestroyBuffer(vkBuffer, allocation);
		});
	}
}
