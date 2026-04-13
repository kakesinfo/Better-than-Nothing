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

#include "RenderingDevice/Handle.hpp"
#include "RenderingDevice/PipelineDescription.hpp"
#include "RenderingDevice/RenderingDevice.hpp"
#include "RenderingDevice_Internals.hpp"

#include <glslang/Include/glslang_c_shader_types.h>
#include <spirv_cross/spirv_cross.hpp>

namespace BetterThanNothing
{
	static VkShaderStageFlagBits Internal_GetShaderStage(const EShaderType type)
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

	static VkFormat Internal_GetVulkanFormat(const VertexType& type)
	{
		switch (type)
		{
			case VertexType::Float:
				return VK_FORMAT_R32_SFLOAT;
			case VertexType::Float2:
				return VK_FORMAT_R32G32_SFLOAT;
			case VertexType::Float3:
				return VK_FORMAT_R32G32B32_SFLOAT;
			case VertexType::Float4:
				return VK_FORMAT_R32G32B32A32_SFLOAT;
			default:
				throw std::runtime_error("Invalid vertex type");
		}
	}

	static std::vector<VkVertexInputBindingDescription> Internal_GetVertexBindingDescription(const VertexLayout& layout)
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = layout.GetStride();
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return { bindingDescription };
	}

	static std::vector<VkVertexInputAttributeDescription> Internal_GetVertexAttributeDescriptions(const VertexLayout& layout)
	{
		const auto attributes = layout.GetAttributes();

		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		attributeDescriptions.reserve(attributes.size());

		for (const auto& attribute : attributes)
		{
			VkVertexInputAttributeDescription vertexAttribute{};
			vertexAttribute.binding = 0;
			vertexAttribute.location = attribute.location;
			vertexAttribute.offset = attribute.offset;
			vertexAttribute.format = Internal_GetVulkanFormat(attribute.type);

			attributeDescriptions.push_back(vertexAttribute);
		}

		return attributeDescriptions;
	}

	static VkSampleCountFlagBits Internal_GetSampleCountFlagBits(const uint32_t sampleCount)
	{
		switch (sampleCount)
		{
			case 64: return VK_SAMPLE_COUNT_64_BIT;
			case 32: return VK_SAMPLE_COUNT_32_BIT;
			case 16: return VK_SAMPLE_COUNT_16_BIT;
			case 8:  return VK_SAMPLE_COUNT_8_BIT;
			case 4:  return VK_SAMPLE_COUNT_4_BIT;
			case 2:  return VK_SAMPLE_COUNT_2_BIT;
			default: return VK_SAMPLE_COUNT_1_BIT;
		}
	}

	static bool Internal_CreatePipeline(RenderingApi::Pipeline* dstPipeline, const PipelineDescription& desc,  RenderingDeviceInternals* self, const RenderSettings* renderSettings)
	{
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		for (const auto shaderHandle : desc.shaders)
		{
			const auto shader = self->shaders.Get(shaderHandle);

			VkPipelineShaderStageCreateInfo shaderStageInfo{};
			shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageInfo.stage = Internal_GetShaderStage(shader->type);
			shaderStageInfo.module = shader->module;
			shaderStageInfo.pName = "main";
			shaderStages.push_back(shaderStageInfo);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		const auto vertexBindings = Internal_GetVertexBindingDescription(desc.vertexLayout);
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindings.size());
		vertexInputInfo.pVertexBindingDescriptions = vertexBindings.data();

		const auto vertexAttributes = Internal_GetVertexAttributeDescriptions(desc.vertexLayout);
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes.size());
		vertexInputInfo.pVertexAttributeDescriptions = vertexAttributes.data();

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
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f;		   // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f;	   // Optional

		// MSAA: sampleShadingEnable FALSE, minSampleShading 0.0f
		// SSAA: sampleShadingEnable TRUE, minSampleShading 1.0f
		// FSR : sampleShadingEnable FALSE, rasterizationSamples VK_SAMPLE_COUNT_1_BIT, minSampleShading 0.0f, alphaToCoverageEnable FALSE

		const auto maxSampleLevel = static_cast<uint32_t>(self->device->GetMaxMsaaSamples());
		const auto samplingLevel = std::min(renderSettings->samplingLevel, maxSampleLevel);

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.alphaToCoverageEnable = VK_TRUE;

		if (renderSettings->samplingMode == ESamplingMode::Msaa)
		{
			multisampling.rasterizationSamples = Internal_GetSampleCountFlagBits(maxSampleLevel);
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.minSampleShading = 0.0f;
		}

		if (renderSettings->samplingMode == ESamplingMode::Ssaa)
		{
			multisampling.rasterizationSamples = Internal_GetSampleCountFlagBits(maxSampleLevel);
			multisampling.sampleShadingEnable = VK_TRUE;
			multisampling.minSampleShading = 1.0f;
		}


		// Fallback: If no color attachment format is defined, we take the one from the Swapchain
		// if (m_params.colorAttachmentFormats.empty())
		const std::vector colorAttachmentFormats = {self->swapChain->GetImageFormat()};

		// Fallback: If no depth attachment format is defined, we take the one from the Swapchain
		// if (m_params.depthAttachmentFormat == VK_FORMAT_UNDEFINED)
		const VkFormat depthAttachmentFormat = self->swapChain->GetDepthFormat();
		const VkFormat stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

		VkPipelineRenderingCreateInfo pipelineRenderingInfo{};
		pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		pipelineRenderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentFormats.size());
		pipelineRenderingInfo.pColorAttachmentFormats = colorAttachmentFormats.data();
		pipelineRenderingInfo.depthAttachmentFormat = depthAttachmentFormat;
		pipelineRenderingInfo.stencilAttachmentFormat = stencilAttachmentFormat;

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
		blendAttachments.resize(colorAttachmentFormats.size(), colorBlendState);

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

		if (!self->pipelineLayouts.contains(desc.layoutName))
			return false;

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
		pipelineInfo.layout = self->pipelineLayouts.at(desc.layoutName)->Handle();
		pipelineInfo.pNext = &pipelineRenderingInfo;
		pipelineInfo.renderPass = VK_NULL_HANDLE; // m_context->vkRenderPass->Handle();
		pipelineInfo.subpass = 0;

		if (vkCreateGraphicsPipelines(self->device->Handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &dstPipeline->pipeline) != VK_SUCCESS)
			return false;

		return true;
	}

	Handle RenderingDevice::CreatePipeline(const PipelineDescription& desc)
	{
		const auto handle = m_self->pipelines.Allocate();

		if (!Internal_CreatePipeline(m_self->pipelines.Get(handle), desc, m_self.get(), m_createInfo.renderSettings))
			throw std::runtime_error("Failed to create pipeline");

		return handle;
	}

	RenderingApi::Pipeline* RenderingDevice::GetPipeline(const Handle& handle)
	{
		if (!m_self->pipelines.Has(handle))
			throw std::runtime_error("Invalid pipeline handle!");

		return m_self->pipelines.Get(handle);
	}

	void RenderingDevice::DestroyPipeline(const Handle& handle)
	{
		if (!m_self->pipelines.Has(handle))
			throw std::runtime_error("Invalid pipeline handle!");

		const RenderingApi::Pipeline* pipeline = m_self->pipelines.Get(handle);
		vkDestroyPipeline(m_self->device->Handle(), pipeline->pipeline, nullptr);
		m_self->pipelines.Destroy(handle);
	}
}
