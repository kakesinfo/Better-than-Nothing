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

#include "Data/Graphics/GlobalUniformData.hpp"
#include "RenderingDevice/RenderingDevice.hpp"
#include "RenderingDevice_Internals.hpp"

#include "../Public/RenderingDevice/DescriptorSetUpdateRequest.hpp"
#include "RenderingApi/Resources/Buffer.hpp"
#include "RenderingApi/Resources/DescriptorSet.hpp"
#include "RenderingApi/Resources/Sampler.hpp"
#include "RenderingApi/Resources/Texture.hpp"
#include "RenderingDevice/Handle.hpp"

#include <Common/Logger.hpp>
#include <Module_Platform/Manager/WindowManager.hpp>
#include <RenderingApi.hpp>
#include <cmath>

namespace BetterThanNothing
{
	bool RenderingDevice::Internal_CreateDescriptorSet(const DescriptorSetLayoutType& type, RenderingApi::DescriptorSet* newDescriptorSet) const
	{
		const auto layout = GetDescriptorSetLayout(type)->Handle();

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_self->descriptorPool->Handle();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		return vkAllocateDescriptorSets(m_self->device->Handle(), &allocInfo, &newDescriptorSet->value) == VK_SUCCESS;
	}

	Handle RenderingDevice::CreateDescriptorSet(DescriptorSetLayoutType layoutType)
	{
		const Handle handle = m_self->descriptors.Allocate();

		if (!Internal_CreateDescriptorSet(layoutType, m_self->descriptors.Get(handle)))
		{
			m_self->descriptors.Destroy(handle);
			throw std::runtime_error("Failed to allocate descriptor set");
		}

		return handle;
	}

	Handle RenderingDevice::CreateTextureDescriptorSet(const Handle& textureHandle, const Handle& samplerHandle)
	{
		const Handle handle = CreateDescriptorSet(DescriptorSetLayoutType::Textures);

		const RenderingApi::DescriptorSet* descriptor = GetDescriptorSet(handle);
		const RenderingApi::Texture*	   texture = GetTexture(textureHandle);
		const RenderingApi::Sampler*	   sampler = GetSampler(samplerHandle);

		DescriptorSetUpdateRequest request{};

		request.imageInfo = {};
		request.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		request.imageInfo.imageView = texture->defaultView;
		request.imageInfo.sampler = sampler->value;

		request.write = {};
		request.write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		request.write.dstSet = descriptor->value;
		request.write.dstBinding = 0;
		request.write.dstArrayElement = 0;
		request.write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		request.write.descriptorCount = 1;
		request.write.pImageInfo = &request.imageInfo;

		RequestDescriptorSetUpdate(request);

		return handle;
	}

	Handle RenderingDevice::CreateUniformBufferDescriptorSet(const Handle& storageBufferHandle, const size_t size, const DescriptorSetLayoutType layoutType)
	{
		const Handle handle = CreateDescriptorSet(layoutType);

		const RenderingApi::DescriptorSet* descriptor = GetDescriptorSet(handle);
		const RenderingApi::Buffer*		   buffer = GetBuffer(storageBufferHandle);

		DescriptorSetUpdateRequest request{};

		request.bufferInfo = {};
		request.bufferInfo.buffer = buffer->value;
		request.bufferInfo.offset = 0;
		request.bufferInfo.range = size;

		request.write = {};
		request.write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		request.write.dstSet = descriptor->value;
		request.write.dstBinding = 0;
		request.write.dstArrayElement = 0;
		request.write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		request.write.descriptorCount = 1;
		request.write.pBufferInfo = &request.bufferInfo;

		RequestDescriptorSetUpdate(request);

		return handle;
	}

	Handle RenderingDevice::CreateStorageBufferDescriptorSet(const Handle& uniformBufferHandle, const size_t size, const DescriptorSetLayoutType layoutType)
	{
		const Handle handle = CreateDescriptorSet(layoutType);

		const RenderingApi::DescriptorSet* descriptor = GetDescriptorSet(handle);
		const RenderingApi::Buffer*		   buffer = GetBuffer(uniformBufferHandle);

		DescriptorSetUpdateRequest request{};

		request.bufferInfo = {};
		request.bufferInfo.buffer = buffer->value;
		request.bufferInfo.offset = 0;
		request.bufferInfo.range = size;

		request.write = {};
		request.write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		request.write.dstSet = descriptor->value;
		request.write.dstBinding = 0;
		request.write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		request.write.descriptorCount = 1;
		request.write.pBufferInfo = &request.bufferInfo;

		RequestDescriptorSetUpdate(request);

		return handle;
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

		for (const auto& request : m_self->descriptorsUpdateRequests)
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

		m_self->descriptors.Destroy(handle);
	}

	RenderingApi::DescriptorSet* RenderingDevice::GetDescriptorSet(const Handle& handle)
	{
		if (!m_self->descriptors.Has(handle))
			throw std::runtime_error("Invalid descriptor set handle!");

		return m_self->descriptors.Get(handle);
	}
}
