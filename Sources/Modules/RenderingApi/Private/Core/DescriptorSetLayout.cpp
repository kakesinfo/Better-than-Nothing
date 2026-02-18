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

#include <RenderingApi.hpp>

namespace BetterThanNothing::RenderingApi
{
	DescriptorSetLayout::DescriptorSetLayout(const DescriptorSetLayoutCreateInfo& createInfo): m_createInfo(createInfo)
	{
		VkDescriptorSetLayoutBinding defaultBinding{};
		defaultBinding.descriptorType = createInfo.descriptorType;
		defaultBinding.descriptorCount = createInfo.descriptorCount;
		defaultBinding.stageFlags = createInfo.stageFlags;

		const std::array bindings = { defaultBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(createInfo.device->Handle(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS)
			throw std::runtime_error("Failed to create descriptor set layout!");
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		if (m_layout != VK_NULL_HANDLE)
			vkDestroyDescriptorSetLayout(m_createInfo.device->Handle(), m_layout, nullptr);
	}

	std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Create(const DescriptorSetLayoutCreateInfo& createInfo)
	{
		return std::make_unique<DescriptorSetLayout>(createInfo);
	}

	VkDescriptorSetLayout DescriptorSetLayout::Handle() const
	{
		return m_layout;
	}
}
