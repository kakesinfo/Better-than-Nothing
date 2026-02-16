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

#pragma once

#include <memory>
#include <vector>

namespace BetterThanNothing
{
	class Device;

	class DescriptorPool;
	class DescriptorPoolBuilder;

	struct DescriptorPoolCreateInfo
	{
		Device*							  device;
		std::vector<VkDescriptorPoolSize> poolSizes;
		uint32_t						  maxSets;
	};

	class DescriptorPool
	{
	public:
		explicit DescriptorPool(const DescriptorPoolCreateInfo& createInfo);
		~DescriptorPool();

		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool& operator=(const DescriptorPool&) = delete;
		DescriptorPool(DescriptorPool&& other) = delete;
		DescriptorPool& operator=(DescriptorPool&& other) = delete;

		[[nodiscard]] static DescriptorPoolBuilder Builder(Device* device);

		[[nodiscard]] VkDescriptorPool Handle() const;

	private:
		DescriptorPoolCreateInfo m_createInfo;
		VkDescriptorPool		 m_descriptorPool = VK_NULL_HANDLE;
	};

	class DescriptorPoolBuilder
	{
	public:
		explicit DescriptorPoolBuilder(Device* device);

		[[nodiscard]] DescriptorPoolBuilder&		  AddPoolSize(VkDescriptorType type, uint32_t count);
		[[nodiscard]] std::unique_ptr<DescriptorPool> Build() const;

	private:
		Device*							  m_device = nullptr;
		std::vector<VkDescriptorPoolSize> m_poolSizes;
		uint32_t						  m_maxSets = 0;
	};
} // BetterThanNothing
