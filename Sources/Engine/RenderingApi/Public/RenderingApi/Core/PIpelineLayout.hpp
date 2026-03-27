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

#include <vulkan/vulkan.h>
#include <vector>

namespace BetterThanNothing::RenderingApi
{
	enum class EDescriptorSetLayoutType : uint8_t;

	struct PipelinePushConstantInfo
	{
		uint32_t stageFlags;
		uint32_t offset;
		uint32_t size;
	};

	struct PipelineLayoutCreateInfo
	{
		std::vector<EDescriptorSetLayoutType> descriptorSetLayouts;
		std::vector<PipelinePushConstantInfo> pushConstantInfos;
	};

	class PipelineLayout
	{
	public:
		explicit PipelineLayout(const PipelineLayoutCreateInfo& createInfo);
		~PipelineLayout();

		PipelineLayout(const PipelineLayout&) = delete;
		PipelineLayout& operator=(const PipelineLayout&) = delete;
		PipelineLayout(PipelineLayout&& other) = delete;
		PipelineLayout& operator=(PipelineLayout&& other) = delete;

		static std::unique_ptr<PipelineLayout> Create(const PipelineLayoutCreateInfo& createInfo);

		[[nodiscard]] VkPipelineLayout Handle() const;

	private:
		VkPipelineLayout m_layout = VK_NULL_HANDLE;
	};
}
