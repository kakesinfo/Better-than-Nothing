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

#include <vector>
#include <vulkan/vulkan.h>

namespace BetterThanNothing::RenderingApi
{
	class DescriptorSetLayout;
	class Device;
	class PipelineLayoutBuilder;

	struct PipelineLayoutCreateInfo
	{
		std::vector<VkDescriptorSetLayout> descriptorLayouts;
		std::vector<VkPushConstantRange>   pushConstants;

		Device* device;
	};

	class PipelineLayout
	{
	public:
		explicit PipelineLayout(const PipelineLayoutCreateInfo& createInfo);
		~PipelineLayout();
		[[nodiscard]] VkPipelineLayout Handle() const;

	private:
		PipelineLayoutCreateInfo m_createInfo;
		Device*					 m_device;
		VkPipelineLayout		 m_pipelineLayout = VK_NULL_HANDLE;
	};
}; // BetterThanNothing
