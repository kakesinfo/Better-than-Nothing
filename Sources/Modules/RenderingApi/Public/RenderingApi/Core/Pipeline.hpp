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

	struct VulkanPipelineShaderInfo
	{
		std::string shaderPath;
		EShaderType shaderType;
	};

	struct VulkanPipelineParams
	{
		const char*							  path;
		const char*							  name;
		std::vector<VulkanPipelineShaderInfo> shadersInfos;

		std::vector<VkFormat> colorAttachmentFormats;
		VkFormat			  depthAttachmentFormat = VK_FORMAT_UNDEFINED;
		VkFormat			  stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
	};

	class Pipeline // TODO: Refactor pipelines to enable building Compute and RayTracing pipelines
	{
	public:
		explicit Pipeline(VulkanPipelineParams params, const VertexInputDescription& vertexInputDescription,
								const std::vector<const ShaderGpuData*>& shaders, VulkanContext* context);
		~Pipeline();

		[[nodiscard]]
		VkPipeline Handle() const;

		[[nodiscard]]
		VkPipelineLayout GetLayout() const;

	private:
		static VkShaderStageFlagBits GetShaderStageTypeByShaderType(EShaderType type);

		VulkanContext*		 m_context = nullptr;
		VulkanPipelineParams m_params;

		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
		VkPipeline		 m_pipeline = VK_NULL_HANDLE;
	};

	class VulkanPipelineBuilder final
	{
	public:
		explicit VulkanPipelineBuilder() = default;
		~VulkanPipelineBuilder() = default;

		VulkanPipelineBuilder& SetPath(const char* path);
		VulkanPipelineBuilder& SetName(const char* name);
		VulkanPipelineBuilder& AddShader(const std::string& shaderPath, EShaderType shaderType);
		VulkanPipelineBuilder& AddVertexShader(const std::string& shaderPath);
		VulkanPipelineBuilder& AddFragmentShader(const std::string& shaderPath);

		VulkanPipelineBuilder& AddColorAttachment(VkFormat format);
		VulkanPipelineBuilder& SetDepthAttachment(VkFormat format);
		VulkanPipelineBuilder& SetStencilAttachment(VkFormat format);

		[[nodiscard]]
		VulkanPipelineParams& GetBuildParams();

	private:
		VulkanPipelineParams m_params{};

		bool m_hasVertexShader = false;
		bool m_hasFragmentShader = false;
	};
}; // BetterThanNothing
