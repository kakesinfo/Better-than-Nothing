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

#include "Renderer/Passes/ImGuiPass.hpp"

#include "Module_Platform/Manager/WindowManager.hpp"
#include "Module_Platform/Window.hpp"
#include "Renderer/RenderMetrics.hpp"
#include "RenderingApi/Core/CommandBuffer.hpp"
#include "RenderingApi/Core/Device.hpp"
#include "RenderingApi/Core/Instance.hpp"
#include "RenderingApi/Core/Queue.hpp"
#include "RenderingApi/Core/SwapChain.hpp"
#include "RenderingApi/Deprecated/VulkanContext.hpp"
#include "RenderingDevice/DrawStream.hpp"

namespace BetterThanNothing
{
	ImGuiPass::ImGuiPass(VulkanContext* context, RenderingDevice* renderingDevice) : m_context(context), m_renderingDevice(renderingDevice)
	{
		const VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000}, {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000}, {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000}, {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000}, {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000}, {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
		pool_info.pPoolSizes = pool_sizes;

		if (vkCreateDescriptorPool(m_context->device->Handle(), &pool_info, nullptr, &m_imguiPool) != VK_SUCCESS)
			throw std::runtime_error("failed to create ImGui descriptor pool!");

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		(void)io;
		ImGui::StyleColorsDark();

		ImGui_ImplSDL3_InitForVulkan(WindowManager::GetWindow()->Handle());

		const std::vector formats = {m_context->swapChain->GetImageFormat()};

		VkPipelineRenderingCreateInfoKHR pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		pipelineInfo.colorAttachmentCount = static_cast<uint32_t>(formats.size());
		pipelineInfo.pColorAttachmentFormats = formats.data();
		pipelineInfo.depthAttachmentFormat = m_context->swapChain->GetDepthFormat();

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.ApiVersion = m_context->instance->GetApiVersion();
		initInfo.Instance = m_context->instance->Handle();
		initInfo.PhysicalDevice = m_context->device->PhysicalHandle();
		initInfo.Device = m_context->device->Handle();
		initInfo.Queue = m_context->graphicsQueue->Handle();
		initInfo.DescriptorPool = m_imguiPool;
		initInfo.MinImageCount = m_context->swapChain->GetImageCount();
		initInfo.ImageCount = m_context->swapChain->GetImageCount();
		// Note: Stencil format can be added if needed
		initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineInfo;
		initInfo.PipelineInfoMain.RenderPass = VK_NULL_HANDLE;
		initInfo.UseDynamicRendering = true;

		ImGui_ImplVulkan_Init(&initInfo);
	}

	ImGuiPass::~ImGuiPass()
	{
		vkDeviceWaitIdle(m_context->device->Handle());

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();

		vkDestroyDescriptorPool(m_context->device->Handle(), m_imguiPool, nullptr);
	}

	void ImGuiPass::Execute(const RenderPassContext& context, const DrawStreamView& drawStream)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		static float updateTimer = 0.0f;
		static auto	 cachedMetrics = RenderMetrics::GetLastFrameMetrics();

		updateTimer += ImGui::GetIO().DeltaTime;
		if (updateTimer > 0.1f)
		{
			cachedMetrics = RenderMetrics::GetLastFrameMetrics();
			updateTimer = 0.0f;
		}

		ImGui::SetNextWindowSize(ImVec2(200, 250), ImGuiCond_Always);
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::Begin("Debug Info", nullptr, ImGuiWindowFlags_NoResize);
		ImGui::SeparatorText("Rendering");
		ImGui::Text("Vertices: %d", cachedMetrics.verticesRendered);
		ImGui::Text("Draw calls: %d", cachedMetrics.drawCalls);
		ImGui::Text("Total draw calls: %zu", cachedMetrics.totalDrawCalls);
		ImGui::SeparatorText("Frames");
		ImGui::Text("Frame count: %d", cachedMetrics.frameCount);
		ImGui::Text("Frame time: %.2f ms", cachedMetrics.frameTime);
		ImGui::Text("FPS: %d", cachedMetrics.fps);
		ImGui::SeparatorText("Camera");
		ImGui::Text("Pos: x=%.1f y=%.1f z=%.1f", cachedMetrics.cameraX, cachedMetrics.cameraY, cachedMetrics.cameraZ);
		ImGui::End();

		ImGui::Render();

		const auto commandBuffer = context.commandBuffer;

		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_context->swapChain->GetImage(context.imageIndex);
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			commandBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		VkRenderingAttachmentInfo colorAttachment = {};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.imageView = m_context->swapChain->GetImageView(context.imageIndex);
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		VkRenderingInfo renderingInfo = {};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea = {{0, 0}, m_context->swapChain->GetExtent()};
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;

		// clang-format off
		commandBuffer->ExecuteRenderPass([&](const RenderingApi::CommandBuffer* cmd) {
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd->Handle());
		}, renderingInfo);
		// clang-format on

		{
			VkImageMemoryBarrier presentationBarrier{};
			presentationBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			presentationBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			presentationBarrier.dstAccessMask = 0;
			presentationBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			presentationBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			presentationBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			presentationBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			presentationBarrier.image = m_context->swapChain->GetImage(context.imageIndex);
			presentationBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			presentationBarrier.subresourceRange.baseMipLevel = 0;
			presentationBarrier.subresourceRange.levelCount = 1;
			presentationBarrier.subresourceRange.baseArrayLayer = 0;
			presentationBarrier.subresourceRange.layerCount = 1;

			commandBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentationBarrier);
		}
	}
}
