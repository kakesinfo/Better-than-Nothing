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

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace BetterThanNothing::RenderingApi
{
	class Device;

	struct DescriptorSetLayoutCreateInfo
	{
		Device*									  device = nullptr;
		VkDescriptorType						  descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uint32_t								  descriptorCount = 1;
		VkShaderStageFlags						  stageFlags = 0;
		std::vector<VkDescriptorSetLayoutBinding> bindings;
	};

	class DescriptorSetLayout
	{
	public:
		explicit DescriptorSetLayout(const DescriptorSetLayoutCreateInfo& createInfo);
		~DescriptorSetLayout();

		DescriptorSetLayout(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout(DescriptorSetLayout&&) = delete;
		DescriptorSetLayout& operator=(DescriptorSetLayout&&) = delete;

		[[nodiscard]] static std::unique_ptr<DescriptorSetLayout> Create(const DescriptorSetLayoutCreateInfo& createInfo);
		[[nodiscard]] VkDescriptorSetLayout						  Handle() const;

	private:
		DescriptorSetLayoutCreateInfo m_createInfo;
		VkDescriptorSetLayout		  m_layout = VK_NULL_HANDLE;
	};
}
