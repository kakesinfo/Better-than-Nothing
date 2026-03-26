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

#include "RenderingApi.hpp"
#include <Data/Graphics/DynamicUniformData.hpp>
#include <iostream>

#include <Common/Logger.hpp>

namespace BetterThanNothing::RenderingApi
{
	Pipeline::Pipeline(const PipelineCreateInfo& params, const std::vector<const ShaderGpuData*>& shaders): m_params(params)
	{
		VkPushConstantRange dynamicPushConstant{};
		dynamicPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		dynamicPushConstant.offset = 0;
		dynamicPushConstant.size = sizeof(DynamicUniformData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(params.descriptorLayouts.size());
		pipelineLayoutInfo.pSetLayouts = params.descriptorLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &dynamicPushConstant;

		if (vkCreatePipelineLayout(m_params.device->Handle(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("Failed to create pipeline layout");

		Logger::Log<SUCCESS>("Vulkan pipeline layout: ok");

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		for (const ShaderGpuData* shaderGpuData : shaders)
		{
			VkShaderStageFlagBits shaderStageType = GetShaderStageTypeByShaderType(shaderGpuData->type);

			VkPipelineShaderStageCreateInfo shaderStageInfo{};
			shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageInfo.stage = shaderStageType;
			shaderStageInfo.module = shaderGpuData->shaderModule->Handle();
			shaderStageInfo.pName = params.name;
			shaderStages.push_back(shaderStageInfo);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = params.vertexInputDescription.GetCreateInfo();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		std::vector dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = nullptr; // Dynamic
		viewportState.scissorCount = 1;
		viewportState.pScissors = nullptr; // Dynamic

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // use VK_POLYGON_MODE_LINE for wireframe
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f;		   // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f;	   // Optional

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.rasterizationSamples = m_params.device->GetMaxMsaaSamples();
		multisampling.sampleShadingEnable = VK_TRUE;
		multisampling.alphaToCoverageEnable = VK_TRUE;
		multisampling.minSampleShading = 0.8f; // closer to one is better quality

		// Fallback: If no color attachment format is defined, we take the one from the Swapchain
		if (m_params.colorAttachmentFormats.empty())
			m_params.colorAttachmentFormats.push_back(m_params.swapChain->GetImageFormat());

		// Fallback: If no depth attachment format is defined, we take the one from the Swapchain
		if (m_params.depthAttachmentFormat == VK_FORMAT_UNDEFINED)
			m_params.depthAttachmentFormat = m_params.swapChain->GetDepthFormat();

		VkPipelineRenderingCreateInfo pipelineRenderingInfo{};
		pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		pipelineRenderingInfo.colorAttachmentCount = static_cast<uint32_t>(m_params.colorAttachmentFormats.size());
		pipelineRenderingInfo.pColorAttachmentFormats = m_params.colorAttachmentFormats.data();
		pipelineRenderingInfo.depthAttachmentFormat = m_params.depthAttachmentFormat;
		pipelineRenderingInfo.stencilAttachmentFormat = m_params.stencilAttachmentFormat;

		VkPipelineColorBlendAttachmentState colorBlendState{};
		colorBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendState.blendEnable = VK_TRUE;
		colorBlendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendState.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendState.alphaBlendOp = VK_BLEND_OP_ADD;

		std::vector<VkPipelineColorBlendAttachmentState> blendAttachments;
		blendAttachments.resize(m_params.colorAttachmentFormats.size(), colorBlendState);

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = static_cast<uint32_t>(blendAttachments.size());
		colorBlending.pAttachments = blendAttachments.data();

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {}; // Optional
		depthStencil.back = {};	 // Optional

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = m_pipelineLayout;
		pipelineInfo.pNext = &pipelineRenderingInfo;
		pipelineInfo.renderPass = VK_NULL_HANDLE; // m_context->vkRenderPass->Handle();
		pipelineInfo.subpass = 0;

		if (vkCreateGraphicsPipelines(m_params.device->Handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS)
			throw std::runtime_error("Failed to create graphics pipeline");

		Logger::Log<SUCCESS>("Vulkan graphics pipeline: ok");
	}

	Pipeline::~Pipeline()
	{
		if (m_pipelineLayout != VK_NULL_HANDLE)
			vkDestroyPipelineLayout(m_params.device->Handle(), m_pipelineLayout, nullptr);

		if (m_pipeline != VK_NULL_HANDLE)
			vkDestroyPipeline(m_params.device->Handle(), m_pipeline, nullptr);
	}

	VkPipeline Pipeline::Handle() const
	{
		return m_pipeline;
	}

	VkPipelineLayout Pipeline::GetLayout() const
	{
		return m_pipelineLayout;
	}

	VkShaderStageFlagBits Pipeline::GetShaderStageTypeByShaderType(const EShaderType type)
	{
		switch (type)
		{
			case EShaderType::Vertex:
				return VK_SHADER_STAGE_VERTEX_BIT;
			case EShaderType::Fragment:
				return VK_SHADER_STAGE_FRAGMENT_BIT;
			default:
				throw std::runtime_error("Invalid shader type");
		}
	}

	PipelineBuilder& PipelineBuilder::SetPath(const char* path)
	{
		m_params.path = path;
		return *this;
	}

	PipelineBuilder& PipelineBuilder::SetName(const char* name)
	{
		m_params.name = name;
		return *this;
	}

	PipelineBuilder& PipelineBuilder::AddShader(const std::string& shaderPath, const EShaderType shaderType)
	{
		PipelineShaderInfo shaderInfo;
		shaderInfo.shaderPath = shaderPath;
		shaderInfo.shaderType = shaderType;

		m_params.shadersInfos.push_back(shaderInfo);

		if (shaderType == EShaderType::Vertex)
			m_hasVertexShader = true;

		if (shaderType == EShaderType::Fragment)
			m_hasFragmentShader = true;

		return *this;
	}

	PipelineBuilder& PipelineBuilder::AddVertexShader(const std::string& shaderPath)
	{
		return AddShader(shaderPath, EShaderType::Vertex);
	}

	PipelineBuilder& PipelineBuilder::AddFragmentShader(const std::string& shaderPath)
	{
		return AddShader(shaderPath, EShaderType::Fragment);
	}

	PipelineBuilder& PipelineBuilder::AddColorAttachment(VkFormat format)
	{
		m_params.colorAttachmentFormats.push_back(format);
		return *this;
	}

	PipelineBuilder& PipelineBuilder::SetDepthAttachment(VkFormat format)
	{
		m_params.depthAttachmentFormat = format;
		return *this;
	}

	PipelineBuilder& PipelineBuilder::SetStencilAttachment(VkFormat format)
	{
		m_params.stencilAttachmentFormat = format;
		return *this;
	}

	PipelineBuilder& PipelineBuilder::AddDescriptorSetLayout(const VkDescriptorSetLayout& layout)
	{
		m_params.descriptorLayouts.push_back(layout);
		return *this;
	}

	PipelineCreateInfo& PipelineBuilder::GetBuildParams()
	{
		if (!m_hasVertexShader)
			throw std::runtime_error("Missing vertex shader");

		if (!m_hasFragmentShader)
			throw std::runtime_error("Missing fragment shader");

		return m_params;
	}
};
