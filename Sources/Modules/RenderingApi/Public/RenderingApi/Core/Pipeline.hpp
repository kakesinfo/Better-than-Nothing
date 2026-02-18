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

#include "Module_Base/EShaderType.hpp"

namespace BetterThanNothing
{
	struct VertexInputDescription;
	struct VulkanContext;
	struct ShaderGpuData;
}

namespace BetterThanNothing::RenderingApi
{
	class DescriptorSetLayout;

	struct PipelineShaderInfo
	{
		std::string shaderPath;
		EShaderType shaderType;
	};

	struct PipelineParams
	{
		const char*						   path;
		const char*						   name;
		std::vector<VkDescriptorSetLayout> descriptorLayouts;
		std::vector<PipelineShaderInfo>	   shadersInfos;

		std::vector<VkFormat> colorAttachmentFormats;
		VkFormat			  depthAttachmentFormat = VK_FORMAT_UNDEFINED;
		VkFormat			  stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
	};

	class Pipeline // TODO: Refactor pipelines to enable building Compute and RayTracing pipelines
	{
	public:
		explicit Pipeline(const PipelineParams& params, const VertexInputDescription& vertexInputDescription, const std::vector<const ShaderGpuData*>& shaders, VulkanContext* context);
		~Pipeline();

		[[nodiscard]] VkPipeline	   Handle() const;
		[[nodiscard]] VkPipelineLayout GetLayout() const;

	private:
		static VkShaderStageFlagBits GetShaderStageTypeByShaderType(EShaderType type);

		VulkanContext* m_context = nullptr;
		PipelineParams m_params;

		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
		VkPipeline		 m_pipeline = VK_NULL_HANDLE;
	};

	class PipelineBuilder final
	{
	public:
		explicit PipelineBuilder() = default;
		~PipelineBuilder() = default;

		PipelineBuilder& SetPath(const char* path);
		PipelineBuilder& SetName(const char* name);
		PipelineBuilder& AddShader(const std::string& shaderPath, EShaderType shaderType);
		PipelineBuilder& AddVertexShader(const std::string& shaderPath);
		PipelineBuilder& AddFragmentShader(const std::string& shaderPath);
		PipelineBuilder& AddDescriptorSetLayout(const VkDescriptorSetLayout& layout);

		PipelineBuilder& AddColorAttachment(VkFormat format);
		PipelineBuilder& SetDepthAttachment(VkFormat format);
		PipelineBuilder& SetStencilAttachment(VkFormat format);

		[[nodiscard]] PipelineParams& GetBuildParams();

	private:
		PipelineParams m_params{};

		bool m_hasVertexShader = false;
		bool m_hasFragmentShader = false;
	};
}; // BetterThanNothing
