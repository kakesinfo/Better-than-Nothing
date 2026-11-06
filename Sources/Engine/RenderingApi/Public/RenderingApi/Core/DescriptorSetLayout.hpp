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
		Device*			   device;
		VkDescriptorType   descriptorType;
		uint32_t		   descriptorCount;
		VkShaderStageFlags stageFlags;
	};

	class DescriptorSetLayout
	{
	public:
		DescriptorSetLayout(const DescriptorSetLayoutCreateInfo& createInfo);
		DescriptorSetLayout(Device* device, const std::vector<VkDescriptorSetLayoutBinding>& bindings);
		~DescriptorSetLayout();

		[[nodiscard]] static std::unique_ptr<DescriptorSetLayout> Create(const DescriptorSetLayoutCreateInfo& createInfo);
		[[nodiscard]] static std::unique_ptr<DescriptorSetLayout> Create(Device* device, const std::vector<VkDescriptorSetLayoutBinding>& bindings);
		[[nodiscard]] VkDescriptorSetLayout						  Handle() const;

	private:
		Device*						  m_device = nullptr;
		VkDescriptorSetLayout		  m_layout = VK_NULL_HANDLE;
	};
}
