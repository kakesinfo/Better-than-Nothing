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

#include "Handle.hpp"
#include "HandleRegistry.hpp"
#include "Types.hpp"

#include <unordered_map>

namespace BetterThanNothing
{
	struct DescriptorSetUpdateRequest;

	class RenderingDevice;

	namespace RenderingApi
	{
		class Device;
		class DescriptorPool;
		struct DescriptorSet;
	}

	struct DescriptorSetRegistryCreateInfo
	{
		RenderingApi::Device*		  device;
		RenderingApi::DescriptorPool* descriptorPool;
		RenderingDevice*			  renderingDevice;
	};

	class DescriptorSetRegistry
	{
	public:
		explicit DescriptorSetRegistry(const DescriptorSetRegistryCreateInfo& createInfo);
		~DescriptorSetRegistry();

		static std::unique_ptr<DescriptorSetRegistry> Create(const DescriptorSetRegistryCreateInfo& createInfo);

	private:
		bool Internal_CreateDescriptorSet(const DescriptorSetLayoutType& type, RenderingApi::DescriptorSet* newDescriptorSet) const;

	public:
		Handle<RenderingApi::DescriptorSet> CreateDescriptorSet(const DescriptorSetLayoutType& layoutType);
		void								DestroyDescriptorSet(Handle<RenderingApi::DescriptorSet> handle);
		RenderingApi::DescriptorSet*		GetDescriptorSet(Handle<RenderingApi::DescriptorSet> handle);
		void								RequestDescriptorSetUpdate(const DescriptorSetUpdateRequest& request);
		void								FlushDescriptorSetUpdateRequests();

	private:
		DescriptorSetRegistryCreateInfo				m_createInfo;
		HandleRegistry<RenderingApi::DescriptorSet> m_descriptors;
		std::vector<DescriptorSetUpdateRequest>		m_updateRequests;
		std::vector<VkWriteDescriptorSet>			m_writeDescriptorSets;
	};
}
