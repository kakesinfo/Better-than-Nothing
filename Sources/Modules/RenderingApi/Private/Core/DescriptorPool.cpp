/*
 *	Better than Nothing
 *	Copyright (C) 2025 kakesinfo
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

#include "RenderingApi/Core/DescriptorPool.hpp"
#include "RenderingApi/Core/Device.hpp"

#include <Module_Base/Logger.hpp>
#include <utility>

namespace BetterThanNothing::RenderingApi
{
	DescriptorPool::DescriptorPool(DescriptorPoolCreateInfo  createInfo) : m_createInfo(std::move(createInfo))
	{
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(m_createInfo.poolSizes.size());
		poolInfo.pPoolSizes = m_createInfo.poolSizes.data();
		poolInfo.maxSets = m_createInfo.maxSets;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		if (vkCreateDescriptorPool(m_createInfo.device->Handle(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
			throw std::runtime_error("Failed to create descriptor pool");

		Logger::Log<INFO>("Vulkan descriptor pool: ok");
	}

	DescriptorPool::~DescriptorPool()
	{
		if (m_descriptorPool != VK_NULL_HANDLE)
			vkDestroyDescriptorPool(m_createInfo.device->Handle(), m_descriptorPool, nullptr);
	}

	DescriptorPoolBuilder DescriptorPool::Builder(Device* device)
	{
		return DescriptorPoolBuilder(device);
	}

	VkDescriptorPool DescriptorPool::Handle() const
	{
		return m_descriptorPool;
	}

	DescriptorPoolBuilder::DescriptorPoolBuilder(Device* device) : m_device(device) {}

	DescriptorPoolBuilder& DescriptorPoolBuilder::AddPoolSize(VkDescriptorType type, uint32_t count)
	{
		m_poolSizes.emplace_back(type, count);
		m_maxSets += count;
		return *this;
	}

	std::unique_ptr<DescriptorPool> DescriptorPoolBuilder::Build() const
	{
		DescriptorPoolCreateInfo createInfo{};
		createInfo.device = m_device;
		createInfo.poolSizes = m_poolSizes;
		createInfo.maxSets = m_maxSets;

		return std::make_unique<DescriptorPool>(createInfo);
	}
}
