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

#pragma once

#include <RenderingApi/Core/DescriptorPool.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace BetterThanNothing
{
	namespace RenderingApi
	{
		class Device;
	}

	/// Growable Vulkan descriptor pool allocator.
	/// Automatically allocates new VkDescriptorPools when the current pool is exhausted or fragmented.
	class DescriptorAllocator
	{
	public:
		explicit DescriptorAllocator(RenderingApi::Device* device, uint32_t setsPerPool = 512);
		~DescriptorAllocator() = default;

		DescriptorAllocator(const DescriptorAllocator&) = delete;
		DescriptorAllocator& operator=(const DescriptorAllocator&) = delete;
		DescriptorAllocator(DescriptorAllocator&&) = delete;
		DescriptorAllocator& operator=(DescriptorAllocator&&) = delete;

		bool Allocate(VkDescriptorSetLayout layout, VkDescriptorSet& outSet, VkDescriptorPool& outOwnerPool);
		void Free(VkDescriptorPool ownerPool, VkDescriptorSet set) const;

	private:
		RenderingApi::DescriptorPool* AcquirePool();
		RenderingApi::DescriptorPool* CreatePool();

		RenderingApi::Device*										 m_device = nullptr;
		uint32_t													 m_setsPerPool = 512;
		std::vector<std::unique_ptr<RenderingApi::DescriptorPool>>	 m_pools;
		size_t														 m_currentPoolIndex = 0;
	};
}
