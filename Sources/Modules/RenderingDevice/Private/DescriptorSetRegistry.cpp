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

#include "RenderingDevice/DescriptorSetRegistry.hpp"

#include "RenderingApi/Core/DescriptorPool.hpp"
#include "RenderingApi/Core/DescriptorSetLayout.hpp"
#include "RenderingDevice/DescriptorSetUpdateRequest.hpp"

#include "RenderingApi/Core/Device.hpp"
#include "RenderingApi/Resources/DescriptorSet.hpp"
#include "RenderingDevice/RenderingDevice.hpp"

namespace BetterThanNothing
{
	DescriptorSetRegistry::DescriptorSetRegistry(const DescriptorSetRegistryCreateInfo& createInfo) : m_createInfo(createInfo) {}

	DescriptorSetRegistry::~DescriptorSetRegistry() = default;

	std::unique_ptr<DescriptorSetRegistry> DescriptorSetRegistry::Create(const DescriptorSetRegistryCreateInfo& createInfo)
	{
		return std::make_unique<DescriptorSetRegistry>(createInfo);
	}

	bool DescriptorSetRegistry::Internal_CreateDescriptorSet(const DescriptorSetLayoutType& type, RenderingApi::DescriptorSet* newDescriptorSet) const
	{
		const auto layout = m_createInfo.renderingDevice->GetDescriptorSetLayout(type)->Handle();

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_createInfo.descriptorPool->Handle();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		return vkAllocateDescriptorSets(m_createInfo.device->Handle(), &allocInfo, &newDescriptorSet->value) == VK_SUCCESS;
	}

	Handle DescriptorSetRegistry::CreateDescriptorSet(const DescriptorSetLayoutType& layoutType)
	{
		const Handle handle = m_descriptors.Allocate();

		if (!Internal_CreateDescriptorSet(layoutType, m_descriptors.Get(handle)))
		{
			m_descriptors.Destroy(handle);
			throw std::runtime_error("Failed to allocate descriptor set");
		}

		return handle;
	}

	void DescriptorSetRegistry::DestroyDescriptorSet(Handle handle)
	{
		if (!m_descriptors.Has(handle))
			throw std::runtime_error("Invalid descriptor set handle!");

		m_descriptors.Destroy(handle);
	}

	RenderingApi::DescriptorSet* DescriptorSetRegistry::GetDescriptorSet(Handle handle)
	{
		if (!m_descriptors.Has(handle))
			throw std::runtime_error("Invalid descriptor set handle!");

		return m_descriptors.Get(handle);
	}

	void DescriptorSetRegistry::RequestDescriptorSetUpdate(const DescriptorSetUpdateRequest& request)
	{
		m_updateRequests.push_back(request);
	}

	void DescriptorSetRegistry::FlushDescriptorSetUpdateRequests()
	{
		if (m_updateRequests.empty())
			return;

		m_writeDescriptorSets.clear();
		m_writeDescriptorSets.reserve(m_updateRequests.size());

		for (const auto& request : m_updateRequests)
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

			m_writeDescriptorSets.push_back(write);
		}

		if (!m_writeDescriptorSets.empty())
		{
			const auto					writesCount = static_cast<std::uint32_t>(m_writeDescriptorSets.size());
			const VkWriteDescriptorSet* writesData = m_writeDescriptorSets.data();

			vkUpdateDescriptorSets(m_createInfo.device->Handle(), writesCount, writesData, 0, nullptr);
		}

		m_updateRequests.clear();
	}
}
