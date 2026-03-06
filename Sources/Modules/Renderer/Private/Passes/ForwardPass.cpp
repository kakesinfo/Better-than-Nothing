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

#include "Renderer/Passes/ForwardPass.hpp"

#include "Module_Data/Graphics/DynamicUniformData.hpp"
#include "RenderingApi/Core/CommandBuffer.hpp"
#include "RenderingApi/Core/SwapChain.hpp"
#include "RenderingApi/Deprecated/VulkanContext.hpp"
#include "RenderingApi/Resources/Buffer.hpp"
#include "RenderingApi/Resources/DescriptorSet.hpp"
#include "RenderingDevice/DrawStream.hpp"
#include "RenderingDevice/RenderingDevice.hpp"

namespace BetterThanNothing
{
	ForwardPass::ForwardPass(VulkanContext* context, RenderingDevice* renderingDevice)
		: m_context(context), m_renderingDevice(renderingDevice)
	{}

	ForwardPass::~ForwardPass() = default;

	void ForwardPass::Execute(const RenderPassContext& context, const DrawStreamView& drawStream)
	{
		{
			std::array<VkImageMemoryBarrier, 3> barriers{};

			barriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barriers[0].srcAccessMask = 0;
			barriers[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barriers[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barriers[0].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[0].image = m_context->swapChain->GetImage(context.imageIndex);
			barriers[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barriers[0].subresourceRange.baseMipLevel = 0;
			barriers[0].subresourceRange.levelCount = 1;
			barriers[0].subresourceRange.baseArrayLayer = 0;
			barriers[0].subresourceRange.layerCount = 1;

			barriers[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barriers[1].srcAccessMask = 0;
			barriers[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			barriers[1].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barriers[1].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[1].image = m_context->swapChain->GetColorImage();
			barriers[1].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barriers[1].subresourceRange.baseMipLevel = 0;
			barriers[1].subresourceRange.levelCount = 1;
			barriers[1].subresourceRange.baseArrayLayer = 0;
			barriers[1].subresourceRange.layerCount = 1;

			barriers[2].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barriers[2].srcAccessMask = 0;
			barriers[2].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			barriers[2].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barriers[2].newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			barriers[2].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[2].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[2].image = m_context->swapChain->GetDepthImage();
			barriers[2].subresourceRange.aspectMask = m_context->swapChain->GetDepthAspectFlags();
			barriers[2].subresourceRange.baseMipLevel = 0;
			barriers[2].subresourceRange.levelCount = 1;
			barriers[2].subresourceRange.baseArrayLayer = 0;
			barriers[2].subresourceRange.layerCount = 1;

			context.commandBuffer->PipelineBarrier(
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr,
				barriers.size(), barriers.data()
			);
		}

		VkRenderingAttachmentInfo colorAttachment{};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.imageView = m_context->swapChain->GetColorImageView();
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
		colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
		colorAttachment.resolveImageView = m_context->swapChain->GetImageView(context.imageIndex);
		colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkRenderingAttachmentInfo depthAttachment{};
		depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depthAttachment.imageView = m_context->swapChain->GetDepthImageView();
		depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};
		depthAttachment.clearValue.depthStencil = {1.0f, 0};

		VkRenderingInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea = {{0, 0}, m_context->swapChain->GetExtent()};
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;
		renderingInfo.pDepthAttachment = &depthAttachment;

		context.commandBuffer->ExecuteRenderPass(
			[&](RenderingApi::CommandBuffer* cmd) {
				cmd->SetViewport(0, 0, m_context->swapChain->GetExtent(), 0, 1);
				cmd->SetScissor(0, 0, m_context->swapChain->GetExtent());

				VkPipeline lastPipeline = VK_NULL_HANDLE;
				Handle lastSamplerHandle = {};

				bool	   boundGlobalDescriptor = false;
				bool	   boundPointLightDescriptor = false;

				for (const DrawPacket& drawPacket : drawStream)
				{
					if (lastPipeline != drawPacket.pipeline)
					{
						context.commandBuffer->BindPipeline(drawPacket.pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
						lastPipeline = drawPacket.pipeline;
					}

					if (!boundGlobalDescriptor)
					{
						context.commandBuffer->BindDescriptorSets(drawPacket.pipelineLayout, 0, 1, &drawPacket.globalDescriptorSet, 0, nullptr);
						boundGlobalDescriptor = true;
					}

					if (!boundPointLightDescriptor)
					{
						context.commandBuffer->BindDescriptorSets(drawPacket.pipelineLayout, 2, 1, &drawPacket.pointLightDescriptorSet, 0, nullptr);
						boundPointLightDescriptor = true;
					}

					if (lastSamplerHandle != drawPacket.samplerDescriptorSet)
					{
						const auto samplerDescriptorSet = m_renderingDevice->GetDescriptorSet(drawPacket.samplerDescriptorSet);
						context.commandBuffer->BindDescriptorSets(drawPacket.pipelineLayout, 1, 1, &samplerDescriptorSet->value, 0, nullptr);
						lastSamplerHandle = drawPacket.samplerDescriptorSet;
					}

					const DynamicUniformData dynamicUniforms = {drawPacket.transform};
					context.commandBuffer->PushConstants(drawPacket.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(DynamicUniformData), &dynamicUniforms);

					const auto vertexBuffer = m_renderingDevice->GetBuffer(drawPacket.vertexBuffer);
					const auto indexBuffer = m_renderingDevice->GetBuffer(drawPacket.indexBuffer);

					const VkBuffer		   vertexBuffers[] = {vertexBuffer->value};
					constexpr VkDeviceSize offsets[] = {0};
					context.commandBuffer->BindVertexBuffers(0, 1, vertexBuffers, offsets);
					context.commandBuffer->BindIndexBuffer(indexBuffer->value, 0, VK_INDEX_TYPE_UINT32);

					context.commandBuffer->DrawIndexed(drawPacket.indexCount, 1, 0, 0, 0);
				}
			},
			renderingInfo
		);
	}
}
