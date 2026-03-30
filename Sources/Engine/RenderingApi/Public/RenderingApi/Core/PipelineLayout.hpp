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

#include "RenderingApi/Data/VertexInputDescription.hpp"

#include <Common/EShaderType.hpp>
#include <memory>
#include <utility>
#include <vector>
#include <vulkan/vulkan.h>

namespace BetterThanNothing
{
	class RenderingDevice;
	enum class DescriptorSetLayoutType;
}

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

		static PipelineLayoutBuilder Builder(RenderingDevice* renderingDevice);

		[[nodiscard]] VkPipelineLayout Handle() const;

	private:
		PipelineLayoutCreateInfo m_createInfo;
		Device*					 m_device;
		VkPipelineLayout		 m_pipelineLayout = VK_NULL_HANDLE;
	};

	class PipelineLayoutBuilder
	{
	public:
		explicit PipelineLayoutBuilder(RenderingDevice* renderingDevice);
		~PipelineLayoutBuilder() = default;

		PipelineLayoutBuilder& AddDescriptorSetLayout(DescriptorSetLayoutType layoutType);

		template <typename T>
		PipelineLayoutBuilder& AddPushConstantLayout(const EShaderType shaderType)
		{
			VkPushConstantRange pushConstant{};
			pushConstant.stageFlags = std::to_underlying(shaderType);
			pushConstant.size = sizeof(T);

			m_pushConstants.push_back(pushConstant);

			return *this;
		}

		std::unique_ptr<PipelineLayout> Build() const;

	private:
		RenderingDevice* m_renderingDevice;
		std::vector<VkDescriptorSetLayout> m_descriptorLayouts;
		std::vector<VkPushConstantRange>   m_pushConstants;
	};
}; // BetterThanNothing
