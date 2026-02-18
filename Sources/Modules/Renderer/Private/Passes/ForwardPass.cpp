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

#include "Renderer/Passes/ForwardPass.hpp"

#include "RenderingApi/Core/CommandBuffer.hpp"
#include "RenderingApi/Core/SwapChain.hpp"
#include "Renderer/DrawStream.hpp"
#include "RenderingApi/Deprecated/VulkanContext.hpp"

namespace BetterThanNothing
{
	ForwardPass::ForwardPass(VulkanContext* context) : m_context(context) {}
	ForwardPass::~ForwardPass() = default;

	void ForwardPass::Execute(const RenderPassContext& context, const DrawStream& drawStream)
	{
		auto commandBuffer = context.commandBuffer;

		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_context->swapChain->GetImage(context.imageIndex);
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			commandBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		VkRenderingAttachmentInfo colorAttachment{};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.imageView = m_context->swapChain->GetColorImageView();
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
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

		commandBuffer->ExecuteRenderPass(
			[&](RenderingApi::CommandBuffer* cmd) {
				cmd->SetViewport(0, 0, m_context->swapChain->GetExtent(), 0, 1);
				cmd->SetScissor(0, 0, m_context->swapChain->GetExtent());

				VkPipeline lastPipeline = VK_NULL_HANDLE;
				bool	   boundGlobalDescriptor = false;
				bool	   boundPointLightDescriptor = false;

				for (uint32_t i = 0; i < drawStream.Size(); i++)
				{
					const DrawCommand& drawCommand = drawStream[i];

					if (lastPipeline != drawCommand.pipeline)
					{
						commandBuffer->BindPipeline(drawCommand.pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
						lastPipeline = drawCommand.pipeline;
					}

					if (!boundGlobalDescriptor)
					{
						commandBuffer->BindDescriptorSets(drawCommand.pipelineLayout, 0, 1, &drawCommand.globalDescriptorSet, 0, nullptr);
						boundGlobalDescriptor = true;
					}

					commandBuffer->BindDescriptorSets(drawCommand.pipelineLayout, 1, 1, &drawCommand.samplerDescriptorSet, 0, nullptr);

					if (!boundPointLightDescriptor)
					{
						commandBuffer->BindDescriptorSets(drawCommand.pipelineLayout, 2, 1, &drawCommand.pointLightDescriptorSet, 0, nullptr);
						boundPointLightDescriptor = true;
					}

					commandBuffer->PushConstants(
						drawCommand.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(DynamicUniformData),
						&drawCommand.dynamicUniformData
					);

					const VkBuffer		   vertexBuffers[] = {drawCommand.vertexBuffer};
					constexpr VkDeviceSize offsets[] = {0};
					commandBuffer->BindVertexBuffers(0, 1, vertexBuffers, offsets);
					commandBuffer->BindIndexBuffer(drawCommand.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

					commandBuffer->DrawIndexed(drawCommand.indexCount, 1, 0, 0, 0);
				}
			},
			renderingInfo
		);
	}
}
