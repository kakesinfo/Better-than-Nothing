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
#include "RenderingDevice_FormatUtils.hpp"
#include "RenderingDevice_Internals.hpp"

#include <RenderingApi.hpp>
#include <cstring>
#include <vector>

namespace BetterThanNothing
{
	VkDeviceSize AlignUpDeviceSize(const VkDeviceSize value, const VkDeviceSize alignment)
	{
		if (alignment <= 1)
			return value;
		return (value + alignment - 1) & ~(alignment - 1);
	}

	Handle RenderingDevice::CreateBlas(const Handle& vertexBuffer, const uint32_t vertexCount, const uint32_t vertexStride, const Handle& indexBuffer, const uint32_t indexCount)
	{
		if (!m_self->device->IsRayQuerySupported() || vertexCount == 0 || indexCount < 3)
			return {};

		if (m_self->uploadRecording)
			Internal_FlushUploadBuffer();

		const RenderingApi::Buffer* vb = GetBuffer(vertexBuffer);
		const RenderingApi::Buffer* ib = GetBuffer(indexBuffer);

		const VkDeviceAddress vbAddress = m_self->device->GetBufferDeviceAddress(vb->value);
		const VkDeviceAddress ibAddress = m_self->device->GetBufferDeviceAddress(ib->value);

		VkAccelerationStructureGeometryTrianglesDataKHR triangles{};
		triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		triangles.vertexData.deviceAddress = vbAddress;
		triangles.vertexStride = vertexStride;
		triangles.maxVertex = vertexCount - 1;
		triangles.indexType = VK_INDEX_TYPE_UINT32;
		triangles.indexData.deviceAddress = ibAddress;

		VkAccelerationStructureGeometryKHR geometry{};
		geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		geometry.geometry.triangles = triangles;

		VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
		buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		buildInfo.geometryCount = 1;
		buildInfo.pGeometries = &geometry;

		const uint32_t primitiveCount = indexCount / 3;

		VkAccelerationStructureBuildSizesInfoKHR buildSizes{};
		buildSizes.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		m_self->device->GetAccelerationStructureBuildSizes(
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			buildInfo,
			&primitiveCount,
			&buildSizes
		);

		const Handle						 handle = m_self->accelerationStructures.Allocate();
		RenderingApi::AccelerationStructure* as = m_self->accelerationStructures.Get(handle);
		as->size = buildSizes.accelerationStructureSize;

		VkBufferCreateInfo asBufferInfo{};
		asBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		asBufferInfo.size = buildSizes.accelerationStructureSize;
		asBufferInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

		VmaAllocationCreateInfo asAllocInfo{};
		asAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		if (m_self->allocator->CreateBuffer(&asBufferInfo, &asAllocInfo, &as->buffer, &as->allocation, nullptr) != VK_SUCCESS)
		{
			m_self->accelerationStructures.Destroy(handle);
			throw std::runtime_error("Failed to create BLAS backing buffer");
		}

		VkAccelerationStructureCreateInfoKHR asCreateInfo{};
		asCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		asCreateInfo.buffer = as->buffer;
		asCreateInfo.size = buildSizes.accelerationStructureSize;
		asCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

		if (m_self->device->CreateAccelerationStructure(asCreateInfo, &as->handle) != VK_SUCCESS)
		{
			m_self->allocator->DestroyBuffer(as->buffer, as->allocation);
			m_self->accelerationStructures.Destroy(handle);
			throw std::runtime_error("Failed to create BLAS acceleration structure");
		}

		const VkDeviceSize scratchAlignment = m_self->device->GetMinAccelerationStructureScratchOffsetAlignment();
		VkBuffer		   scratchBuffer = VK_NULL_HANDLE;
		VmaAllocation	   scratchAllocation = VK_NULL_HANDLE;

		VkBufferCreateInfo scratchBufferInfo{};
		scratchBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		scratchBufferInfo.size = buildSizes.buildScratchSize + scratchAlignment;
		scratchBufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

		VmaAllocationCreateInfo scratchAllocInfo{};
		scratchAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		if (m_self->allocator->CreateBuffer(&scratchBufferInfo, &scratchAllocInfo, &scratchBuffer, &scratchAllocation, nullptr) != VK_SUCCESS)
		{
			m_self->device->DestroyAccelerationStructure(as->handle);
			m_self->allocator->DestroyBuffer(as->buffer, as->allocation);
			m_self->accelerationStructures.Destroy(handle);
			throw std::runtime_error("Failed to create BLAS scratch buffer");
		}

		const VkDeviceAddress alignedScratchAddr = AlignUpDeviceSize(m_self->device->GetBufferDeviceAddress(scratchBuffer), scratchAlignment);

		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.dstAccelerationStructure = as->handle;
		buildInfo.scratchData.deviceAddress = alignedScratchAddr;

		VkAccelerationStructureBuildRangeInfoKHR rangeInfo{};
		rangeInfo.primitiveCount = primitiveCount;
		rangeInfo.primitiveOffset = 0;
		rangeInfo.firstVertex = 0;
		rangeInfo.transformOffset = 0;
		const VkAccelerationStructureBuildRangeInfoKHR* pRangeInfo = &rangeInfo;

		RenderingApi::CommandBuffer::SingleTimeCommands(
			[&](std::unique_ptr<RenderingApi::CommandBuffer>& cmd) {
				m_self->device->CmdBuildAccelerationStructures(cmd->Handle(), 1, &buildInfo, &pRangeInfo);
			},
			m_self->device.get(),
			m_self->commandPool.get(),
			m_self->graphicsQueue.get()
		);

		m_self->allocator->DestroyBuffer(scratchBuffer, scratchAllocation);
		as->deviceAddress = m_self->device->GetAccelerationStructureDeviceAddress(as->handle);

		return handle;
	}

	void RenderingDevice::DestroyAccelerationStructure(const Handle& handle)
	{
		if (!m_self->accelerationStructures.Has(handle))
			throw std::runtime_error("Invalid acceleration structure handle!");

		const RenderingApi::AccelerationStructure* as = m_self->accelerationStructures.Get(handle);
		const VkAccelerationStructureKHR		   vkAs = as->handle;
		const VkBuffer							   vkBuffer = as->buffer;
		const VmaAllocation						   allocation = as->allocation;
		m_self->accelerationStructures.Destroy(handle);

		RenderingApi::Device*	 device = m_self->device.get();
		RenderingApi::Allocator* allocator = m_self->allocator.get();
		Internal_EnqueueDeletion([device, allocator, vkAs, vkBuffer, allocation]() {
			device->DestroyAccelerationStructure(vkAs);
			allocator->DestroyBuffer(vkBuffer, allocation);
		});
	}

	RenderingApi::AccelerationStructure* RenderingDevice::GetAccelerationStructure(const Handle& handle)
	{
		if (!m_self->accelerationStructures.Has(handle))
			throw std::runtime_error("Invalid acceleration structure handle!");
		return m_self->accelerationStructures.Get(handle);
	}

	void RenderingDevice::BuildOrUpdateTlas(Handle& outTlasHandle, Handle& ioScratchBuffer, Handle& ioInstanceBuffer, const std::span<const TlasInstanceDesc> instances)
	{
		if (!m_self->device->IsRayQuerySupported() || instances.empty())
			return;

		std::vector<VkAccelerationStructureInstanceKHR> vkInstances;
		vkInstances.reserve(instances.size());
		for (const auto& inst : instances)
		{
			const RenderingApi::AccelerationStructure* blas = GetAccelerationStructure(inst.blasHandle);

			VkAccelerationStructureInstanceKHR vkInst{};
			std::memcpy(vkInst.transform.matrix, inst.transform, sizeof(vkInst.transform.matrix));
			vkInst.instanceCustomIndex = inst.instanceCustomIndex;
			vkInst.mask = inst.mask;
			vkInst.instanceShaderBindingTableRecordOffset = 0;
			vkInst.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
			vkInst.accelerationStructureReference = blas->deviceAddress;
			vkInstances.push_back(vkInst);
		}

		const VkDeviceSize requiredInstanceBytes = sizeof(VkAccelerationStructureInstanceKHR) * vkInstances.size();
		if (!ioInstanceBuffer.IsValid() || GetBuffer(ioInstanceBuffer)->size < requiredInstanceBytes)
		{
			if (ioInstanceBuffer.IsValid())
				DestroyBuffer(ioInstanceBuffer);

			BufferDescription instDesc{};
			instDesc.size = requiredInstanceBytes;
			instDesc.bufferUsage = BufferUsageFlags::AccelerationStructureBuildInputReadOnly | BufferUsageFlags::ShaderDeviceAddress;
			instDesc.memoryUsage = BufferMemoryUsage::CpuToGpu;
			instDesc.allocationFlags = BufferAllocationFlags::Mapped | BufferAllocationFlags::HostAccessSequentialWrite;
			ioInstanceBuffer = CreateBuffer(instDesc);
		}

		std::memcpy(GetBufferData(ioInstanceBuffer), vkInstances.data(), requiredInstanceBytes);
		const VkDeviceAddress instanceBufferAddress = m_self->device->GetBufferDeviceAddress(GetBuffer(ioInstanceBuffer)->value);

		VkAccelerationStructureGeometryInstancesDataKHR instancesData{};
		instancesData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		instancesData.arrayOfPointers = VK_FALSE;
		instancesData.data.deviceAddress = instanceBufferAddress;

		VkAccelerationStructureGeometryKHR geometry{};
		geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		geometry.geometry.instances = instancesData;

		VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
		buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		buildInfo.geometryCount = 1;
		buildInfo.pGeometries = &geometry;

		const auto primitiveCount = static_cast<uint32_t>(vkInstances.size());

		VkAccelerationStructureBuildSizesInfoKHR buildSizes{};
		buildSizes.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		m_self->device->GetAccelerationStructureBuildSizes(
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			buildInfo,
			&primitiveCount,
			&buildSizes
		);

		const VkDeviceSize scratchAlignment = m_self->device->GetMinAccelerationStructureScratchOffsetAlignment();
		const VkDeviceSize requiredScratchBytes = buildSizes.buildScratchSize + scratchAlignment;
		if (!ioScratchBuffer.IsValid() || GetBuffer(ioScratchBuffer)->size < requiredScratchBytes)
		{
			if (ioScratchBuffer.IsValid())
				DestroyBuffer(ioScratchBuffer);

			BufferDescription scratchDesc{};
			scratchDesc.size = requiredScratchBytes;
			scratchDesc.bufferUsage = BufferUsageFlags::Storage | BufferUsageFlags::ShaderDeviceAddress;
			scratchDesc.memoryUsage = BufferMemoryUsage::GpuOnly;
			ioScratchBuffer = CreateBuffer(scratchDesc);
		}

		if (!outTlasHandle.IsValid() || GetAccelerationStructure(outTlasHandle)->size < buildSizes.accelerationStructureSize)
		{
			if (outTlasHandle.IsValid())
				DestroyAccelerationStructure(outTlasHandle);

			const Handle						 handle = m_self->accelerationStructures.Allocate();
			RenderingApi::AccelerationStructure* tlas = m_self->accelerationStructures.Get(handle);
			tlas->size = buildSizes.accelerationStructureSize;

			VkBufferCreateInfo tlasBufferInfo{};
			tlasBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			tlasBufferInfo.size = buildSizes.accelerationStructureSize;
			tlasBufferInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

			VmaAllocationCreateInfo tlasAllocInfo{};
			tlasAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

			if (m_self->allocator->CreateBuffer(&tlasBufferInfo, &tlasAllocInfo, &tlas->buffer, &tlas->allocation, nullptr) != VK_SUCCESS)
			{
				m_self->accelerationStructures.Destroy(handle);
				throw std::runtime_error("Failed to create TLAS backing buffer");
			}

			VkAccelerationStructureCreateInfoKHR tlasCreateInfo{};
			tlasCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
			tlasCreateInfo.buffer = tlas->buffer;
			tlasCreateInfo.size = buildSizes.accelerationStructureSize;
			tlasCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

			if (m_self->device->CreateAccelerationStructure(tlasCreateInfo, &tlas->handle) != VK_SUCCESS)
			{
				m_self->allocator->DestroyBuffer(tlas->buffer, tlas->allocation);
				m_self->accelerationStructures.Destroy(handle);
				throw std::runtime_error("Failed to create TLAS acceleration structure");
			}

			tlas->deviceAddress = m_self->device->GetAccelerationStructureDeviceAddress(tlas->handle);
			outTlasHandle = handle;
		}

		const RenderingApi::AccelerationStructure* tlas = GetAccelerationStructure(outTlasHandle);
		const VkDeviceAddress					   alignedScratchAddr = AlignUpDeviceSize(m_self->device->GetBufferDeviceAddress(GetBuffer(ioScratchBuffer)->value), scratchAlignment);

		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.dstAccelerationStructure = tlas->handle;
		buildInfo.scratchData.deviceAddress = alignedScratchAddr;

		VkAccelerationStructureBuildRangeInfoKHR rangeInfo{};
		rangeInfo.primitiveCount = primitiveCount;
		rangeInfo.primitiveOffset = 0;
		rangeInfo.firstVertex = 0;
		rangeInfo.transformOffset = 0;
		const VkAccelerationStructureBuildRangeInfoKHR* pRangeInfo = &rangeInfo;

		const auto cmd = m_self->commandBuffers[m_currentFrameIndex].get();
		m_self->device->CmdBuildAccelerationStructures(cmd->Handle(), 1, &buildInfo, &pRangeInfo);

		VkMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;

		VkDependencyInfo depInfo{};
		depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		depInfo.memoryBarrierCount = 1;
		depInfo.pMemoryBarriers = &barrier;

		cmd->PipelineBarrier2(depInfo);
	}
}
