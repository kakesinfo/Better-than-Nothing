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

#include "RenderingApi/Resources/AccelerationStructure.hpp"
#include "RenderingApi/Resources/Buffer.hpp"
#include "RenderingApi/Resources/DescriptorSet.hpp"
#include "RenderingApi/Resources/Sampler.hpp"
#include "RenderingApi/Resources/Texture.hpp"
#include "RenderingDevice/DescriptorSetUpdateRequest.hpp"
#include "RenderingDevice/Handle.hpp"
#include "RenderingDevice/RenderingDevice.hpp"
#include "RenderingDevice_Internals.hpp"
#include <Common/Logger.hpp>
#include <RenderingApi.hpp>

namespace BetterThanNothing
{
	bool RenderingDevice::Internal_CreateDescriptorSet(const std::string_view layoutType, RenderingApi::DescriptorSet* newDescriptorSet) const
	{
		const VkDescriptorSetLayout layout = GetDescriptorSetLayout(std::string(layoutType))->Handle();
		return m_self->descriptorAllocator->Allocate(layout, newDescriptorSet->value, newDescriptorSet->ownerPool);
	}

	Handle RenderingDevice::CreateDescriptorSet(const std::string_view layoutType)
	{
		const Handle handle = m_self->descriptors.Allocate();

		if (!Internal_CreateDescriptorSet(layoutType, m_self->descriptors.Get(handle)))
		{
			m_self->descriptors.Destroy(handle);
			throw std::runtime_error("Failed to allocate descriptor set");
		}

		return handle;
	}

	Handle RenderingDevice::CreateTextureDescriptorSet(const Handle& textureHandle, const Handle& samplerHandle, const std::string_view layoutType)
	{
		const TextureBindingUpdate binding{.binding = 0, .textureHandle = textureHandle, .samplerHandle = samplerHandle};
		return CreateCombinedImageDescriptorSet(std::span<const TextureBindingUpdate>(&binding, 1), layoutType);
	}

	Handle RenderingDevice::CreateCombinedImageDescriptorSet(const std::span<const TextureBindingUpdate> bindings, const std::string_view layoutType)
	{
		const Handle handle = CreateDescriptorSet(layoutType);
		for (const auto& b : bindings)
			UpdateDescriptorImage(handle, b.binding, b.textureHandle, b.samplerHandle);
		return handle;
	}

	Handle RenderingDevice::CreateUniformBufferDescriptorSet(const Handle& uniformBufferHandle, const size_t size, const std::string_view layoutType)
	{
		const Handle handle = CreateDescriptorSet(layoutType);
		UpdateDescriptorBuffer(handle, 0, uniformBufferHandle, size, DescriptorType::UniformBuffer);
		return handle;
	}

	Handle RenderingDevice::CreateStorageBufferDescriptorSet(const Handle& storageBufferHandle, const size_t size, const std::string_view layoutType)
	{
		const Handle handle = CreateDescriptorSet(layoutType);
		UpdateDescriptorBuffer(handle, 0, storageBufferHandle, size, DescriptorType::StorageBuffer);
		return handle;
	}

	void RenderingDevice::UpdateDescriptorBuffer(const Handle& setHandle, const uint32_t binding, const Handle& bufferHandle, const size_t size, const DescriptorType type)
	{
		const RenderingApi::DescriptorSet* descriptor = GetDescriptorSet(setHandle);
		const RenderingApi::Buffer*		   buffer = GetBuffer(bufferHandle);

		DescriptorSetUpdateRequest request{};
		request.bufferInfo.buffer = buffer->value;
		request.bufferInfo.offset = 0;
		request.bufferInfo.range = size;

		request.write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		request.write.dstSet = descriptor->value;
		request.write.dstBinding = binding;
		request.write.dstArrayElement = 0;
		request.write.descriptorType = (type == DescriptorType::StorageBuffer) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		request.write.descriptorCount = 1;

		RequestDescriptorSetUpdate(request);
	}

	void RenderingDevice::UpdateDescriptorImage(const Handle& setHandle, const uint32_t binding, const Handle& textureHandle, const Handle& samplerHandle)
	{
		const RenderingApi::DescriptorSet* descriptor = GetDescriptorSet(setHandle);
		const RenderingApi::Texture*	   texture = GetTexture(textureHandle);
		const RenderingApi::Sampler*	   sampler = GetSampler(samplerHandle);

		DescriptorSetUpdateRequest request{};
		request.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		request.imageInfo.imageView = texture->defaultView;
		request.imageInfo.sampler = sampler->value;

		request.write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		request.write.dstSet = descriptor->value;
		request.write.dstBinding = binding;
		request.write.dstArrayElement = 0;
		request.write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		request.write.descriptorCount = 1;

		RequestDescriptorSetUpdate(request);
	}

	void RenderingDevice::UpdateDescriptorStorageImage(const Handle& setHandle, const uint32_t binding, const Handle& textureHandle)
	{
		const RenderingApi::DescriptorSet* descriptor = GetDescriptorSet(setHandle);
		const RenderingApi::Texture*	   texture = GetTexture(textureHandle);

		DescriptorSetUpdateRequest request{};
		request.imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		request.imageInfo.imageView = texture->defaultView;
		request.imageInfo.sampler = VK_NULL_HANDLE;

		request.write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		request.write.dstSet = descriptor->value;
		request.write.dstBinding = binding;
		request.write.dstArrayElement = 0;
		request.write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		request.write.descriptorCount = 1;

		RequestDescriptorSetUpdate(request);
	}

	void RenderingDevice::UpdateDescriptorAccelerationStructure(const Handle& setHandle, const uint32_t binding, const Handle& asHandle)
	{
		const RenderingApi::DescriptorSet*		   descriptor = GetDescriptorSet(setHandle);
		const RenderingApi::AccelerationStructure* as = m_self->accelerationStructures.Get(asHandle);

		DescriptorSetUpdateRequest request{};
		request.asHandle = as->handle;
		request.asInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		request.asInfo.accelerationStructureCount = 1;

		request.write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		request.write.dstSet = descriptor->value;
		request.write.dstBinding = binding;
		request.write.dstArrayElement = 0;
		request.write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		request.write.descriptorCount = 1;

		RequestDescriptorSetUpdate(request);
	}

	void RenderingDevice::RequestDescriptorSetUpdate(const DescriptorSetUpdateRequest& request)
	{
		m_self->descriptorsUpdateRequests.push_back(request);
	}

	void RenderingDevice::FlushDescriptorSetUpdateRequests()
	{
		if (m_self->descriptorsUpdateRequests.empty())
			return;

		m_self->descriptorsWrites.clear();
		m_self->descriptorsWrites.reserve(m_self->descriptorsUpdateRequests.size());

		for (auto& request : m_self->descriptorsUpdateRequests)
		{
			VkWriteDescriptorSet write = request.write;

			if (write.descriptorCount > 1)
				throw std::runtime_error("Cannot update more than one descriptor set at once");

			switch (write.descriptorType)
			{
				case VK_DESCRIPTOR_TYPE_SAMPLER:
				case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
				case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
				case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
				case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
					write.pImageInfo = &request.imageInfo;
					break;

				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
				case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
					write.pBufferInfo = &request.bufferInfo;
					break;

				case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
					request.asInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
					request.asInfo.pNext = nullptr;
					request.asInfo.accelerationStructureCount = 1;
					request.asInfo.pAccelerationStructures = &request.asHandle;
					write.pNext = &request.asInfo;
					break;

				default:
					throw std::runtime_error("Invalid descriptor type!");
			}

			m_self->descriptorsWrites.push_back(write);
		}

		if (!m_self->descriptorsWrites.empty())
		{
			const auto					writesCount = static_cast<std::uint32_t>(m_self->descriptorsWrites.size());
			const VkWriteDescriptorSet* writesData = m_self->descriptorsWrites.data();

			vkUpdateDescriptorSets(m_self->device->Handle(), writesCount, writesData, 0, nullptr);
		}

		m_self->descriptorsUpdateRequests.clear();
	}

	void RenderingDevice::DestroyDescriptorSet(const Handle& handle)
	{
		if (!m_self->descriptors.Has(handle))
			throw std::runtime_error("Invalid descriptor set handle!");

		const RenderingApi::DescriptorSet* descriptor = m_self->descriptors.Get(handle);
		const VkDescriptorSet			   vkSet = descriptor->value;
		const VkDescriptorPool			   ownerPool = descriptor->ownerPool;
		m_self->descriptors.Destroy(handle);

		if (vkSet != VK_NULL_HANDLE)
		{
			std::erase_if(m_self->descriptorsUpdateRequests, [&](const DescriptorSetUpdateRequest& req) { return req.write.dstSet == vkSet; });

			DescriptorAllocator* allocator = m_self->descriptorAllocator.get();
			Internal_EnqueueDeletion([allocator, ownerPool, vkSet]() {
				allocator->Free(ownerPool, vkSet);
			});
		}
	}

	RenderingApi::DescriptorSet* RenderingDevice::GetDescriptorSet(const Handle& handle)
	{
		if (!m_self->descriptors.Has(handle))
			throw std::runtime_error("Invalid descriptor set handle!");

		return m_self->descriptors.Get(handle);
	}
}
