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

#include <Renderer/ImGuiPass.hpp>

#include <Editor/Editor.hpp>
#include <Module_Base/Metrics.hpp>
#include <Module_Platform/Window.hpp>
#include <RenderingApi/Core/CommandBuffer.hpp>
#include <RenderingApi/Core/SwapChain.hpp>
#include <RenderingDevice/RenderingDevice.hpp>

namespace BetterThanNothing
{
	ImGuiPass::ImGuiPass(RenderingDevice* renderingDevice, Editor* debugUI) : m_renderingDevice(renderingDevice), m_debugUI(debugUI) {}

	void ImGuiPass::Execute(uint32_t imageIndex) const
	{
		const auto commandBuffer = m_renderingDevice->GetCurrentCommandBuffer();
		const auto swapChain = m_renderingDevice->GetSwapChain();

		m_debugUI->RenderPanels();

		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = swapChain->GetImage(imageIndex);
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			commandBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		VkRenderingAttachmentInfo colorAttachment = {};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.imageView = swapChain->GetImageView(imageIndex);
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		VkRenderingInfo renderingInfo = {};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea = {{0, 0}, swapChain->GetExtent()};
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;

		commandBuffer->ExecuteRenderPass([&](const RenderingApi::CommandBuffer* cmd) { m_debugUI->RenderDrawData(cmd); }, renderingInfo);

		{
			VkImageMemoryBarrier presentationBarrier{};
			presentationBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			presentationBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			presentationBarrier.dstAccessMask = 0;
			presentationBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			presentationBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			presentationBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			presentationBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			presentationBarrier.image = swapChain->GetImage(imageIndex);
			presentationBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			presentationBarrier.subresourceRange.baseMipLevel = 0;
			presentationBarrier.subresourceRange.levelCount = 1;
			presentationBarrier.subresourceRange.baseArrayLayer = 0;
			presentationBarrier.subresourceRange.layerCount = 1;

			commandBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentationBarrier);
		}
	}
}
