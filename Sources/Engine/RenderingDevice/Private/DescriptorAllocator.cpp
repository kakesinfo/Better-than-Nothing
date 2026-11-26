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

#include "DescriptorAllocator.hpp"

#include <Common/Logger.hpp>
#include <RenderingApi/Core/Device.hpp>

namespace BetterThanNothing
{
	DescriptorAllocator::DescriptorAllocator(RenderingApi::Device* device, const uint32_t setsPerPool)
		: m_device(device), m_setsPerPool(setsPerPool)
	{
		CreatePool();
	}

	RenderingApi::DescriptorPool* DescriptorAllocator::CreatePool()
	{
		auto builder = RenderingApi::DescriptorPool::Builder(m_device)
						   .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_setsPerPool)
						   .AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, m_setsPerPool)
						   .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_setsPerPool * 2)
						   .AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, m_setsPerPool / 2);
		if (m_device->IsRayQuerySupported())
			static_cast<void>(builder.AddPoolSize(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, std::max(1u, m_setsPerPool / 4)));

		auto pool = builder.Build();

		m_pools.push_back(std::move(pool));
		m_currentPoolIndex = m_pools.size() - 1;
		return m_pools.back().get();
	}

	RenderingApi::DescriptorPool* DescriptorAllocator::AcquirePool()
	{
		if (m_pools.empty())
			return CreatePool();
		return m_pools[m_currentPoolIndex].get();
	}

	bool DescriptorAllocator::Allocate(VkDescriptorSetLayout layout, VkDescriptorSet& outSet, VkDescriptorPool& outOwnerPool)
	{
		RenderingApi::DescriptorPool* pool = AcquirePool();

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool->Handle();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		VkResult result = vkAllocateDescriptorSets(m_device->Handle(), &allocInfo, &outSet);
		if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
		{
			Logger::Log<INFO>("DescriptorAllocator: growing descriptor pool (pool #{})", m_pools.size() + 1);
			pool = CreatePool();
			allocInfo.descriptorPool = pool->Handle();
			result = vkAllocateDescriptorSets(m_device->Handle(), &allocInfo, &outSet);
		}

		if (result != VK_SUCCESS)
		{
			outSet = VK_NULL_HANDLE;
			outOwnerPool = VK_NULL_HANDLE;
			return false;
		}

		outOwnerPool = pool->Handle();
		return true;
	}

	void DescriptorAllocator::Free(VkDescriptorPool ownerPool, VkDescriptorSet set) const
	{
		if (ownerPool == VK_NULL_HANDLE || set == VK_NULL_HANDLE)
			return;
		vkFreeDescriptorSets(m_device->Handle(), ownerPool, 1, &set);
	}
}
