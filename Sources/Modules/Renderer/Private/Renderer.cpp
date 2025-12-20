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

#include <module_vulkan.hpp>

#include "Module_Renderer/Renderer.hpp"

#include "Module_Renderer/Data/CameraData.hpp"
#include "Module_Renderer/Data/DirectionalLightData.hpp"
#include "Module_Renderer/Data/DynamicUniformData.hpp"
#include "Module_Renderer/Data/GlobalUniformData.hpp"
#include "Module_Renderer/Data/ShaderData.hpp"
#include "Module_Renderer/DynamicUniformProvider.hpp"
#include "Module_Renderer/GlobalUniformProvider.hpp"
#include "Module_Renderer/RenderCache.hpp"
#include "Module_Renderer/RenderList.hpp"

#include <Module_Base/Logger.hpp>
#include <Module_Data/Graphics/VertexData.hpp>
#include <Module_Vulkan/VertexInputDescription.hpp>

#include <Module_Platform/Manager/WindowManager.hpp>
#include <Module_Platform/Window.hpp>

namespace BetterThanNothing
{
	Renderer::Renderer(VulkanContext* context) : m_context(context)
	{
		BufferManager::Initialize(m_context);

		Logger::Log<INFO>("Buffering mode: {}", m_context->vkSettings.maxFramesInFlight);
		Logger::Log<INFO>("VSync: {}", m_context->vkSettings.enableVSync ? "enabled" : "disabled");

		m_context->vkInstance = std::make_unique<VulkanInstance>(m_context);
		m_context->vkSurface = std::make_unique<VulkanSurface>(m_context);
		m_context->vkDevice = std::make_unique<VulkanDevice>(m_context);
		m_context->vkMemoryAllocator = std::make_unique<VulkanMemoryAllocator>(m_context);

		Logger::Log<INFO>("Vendor: {}", m_context->vkDevice->GetVendorName());
		Logger::Log<INFO>("Device: {}", m_context->vkDevice->GetDeviceName());
		Logger::Log<INFO>("API Version: {}", m_context->vkDevice->GetApiVersion());

		Logger::Log<INFO>("Anisotropy: {}x", m_context->vkDevice->GetMaxAnisotropy());
		Logger::Log<INFO>("Msaa: {}x", static_cast<std::uint32_t>(m_context->vkDevice->GetMaxMsaaSamples()));

		m_context->vkCommandPool = std::make_unique<VulkanCommandPool>(m_context);

		m_context->vkSwapChain = std::make_unique<VulkanSwapChain>(m_context);
		m_context->vkSwapChain->CreateImages();
		m_context->vkSwapChain->CreateImageViews();
		m_context->vkSwapChain->CreateColorResources();
		m_context->vkSwapChain->CreateDepthResources();

		m_context->vkRenderPass = std::make_unique<VulkanRenderPass>(m_context);

		m_context->vkSwapChain->CreateFramebuffers();

		m_context->vkFrames.resize(m_context->vkSettings.maxFramesInFlight);

		m_globalUniformProvider = std::make_unique<GlobalUniformProvider>(m_context);
		m_dynamicUniformProvider = std::make_unique<DynamicUniformProvider>(m_context, 100000);

		CreateDescriptorSetLayout();
		CreateDescriptorPool();
		CreateGlobalDescriptorSets();
		CreateDynamicDescriptorSets();

		CreateCommandBuffers();
		CreateSyncObjects();

		Logger::Log<SUCCESS>("Renderer: initialized successfully");
	}

	Renderer::~Renderer()
	{
		m_context->vkDevice->WaitIdle();
		BufferManager::Shutdown();
	}

	void Renderer::RenderFrame(RenderList* renderList, const CameraData& camera, const DirectionalLightData& directionalLight)
	{
		if (!m_context->vkPipeline)
			throw std::runtime_error("No pipeline set for rendering!");

		m_context->vkFrames[m_currentFrame].inFlightFence->Wait();

		const auto device = m_context->vkDevice->Handle();
		const auto swapchain = m_context->vkSwapChain->Handle();

		std::uint32_t  imageIndex;
		const VkResult acquireResult = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, m_context->vkFrames[m_currentFrame].imageAvailableSemaphore->Handle(), VK_NULL_HANDLE, &imageIndex);

		if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_context->vkSwapChain->RecreateSwapchain();
			return;
		}

		if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("failed to acquire swap chain image!");

		m_context->vkFrames[m_currentFrame].inFlightFence->Reset();
		m_context->vkFrames[m_currentFrame].commandBuffer->Reset();

		RecordCommandBuffer(renderList, camera, directionalLight, imageIndex);

		const VkSemaphore waitSemaphores[] = {m_context->vkFrames[m_currentFrame].imageAvailableSemaphore->Handle()};

		// We wait for the fragment shader to output the color before we can start writing to the image
		constexpr VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;

		const auto commandBuffer = m_context->vkFrames[m_currentFrame].commandBuffer->Handle();
		submitInfo.pCommandBuffers = &commandBuffer;

		const VkSemaphore signalSemaphores[] = {m_context->vkFrames[m_currentFrame].renderFinishedSemaphore->Handle()};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		auto graphicsQueue = m_context->vkGraphicsQueue->Handle();
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, m_context->vkFrames[m_currentFrame].inFlightFence->Handle()) != VK_SUCCESS)
			throw std::runtime_error("failed to submit draw command buffer!");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		const VkSwapchainKHR swapChains[] = {swapchain};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		const VkResult presentResult = vkQueuePresentKHR(m_context->vkPresentQueue->Handle(), &presentInfo);

		auto& window = WindowManager::GetWindow();
		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || window->IsResized())
		{
			window->SetResized(false);
			m_context->vkSwapChain->RecreateSwapchain();
		}
		else if (presentResult != VK_SUCCESS)
			throw std::runtime_error("failed to present swap chain image!");

		m_currentFrame = (m_currentFrame + 1) % m_context->vkFrames.size();
	}

	void Renderer::RecordCommandBuffer(const RenderList* renderList, const CameraData& cameraData, const DirectionalLightData& directionalLight, const std::uint32_t imageIndex) const
	{
		const auto& currentFrame = m_context->vkFrames[m_currentFrame];

		currentFrame.commandBuffer->BeginRecording();

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_context->vkRenderPass->Handle();
		renderPassInfo.framebuffer = m_context->vkSwapChain->GetFramebuffer(imageIndex);
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = m_context->vkSwapChain->GetExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
		clearValues[1].depthStencil = {1.0f, 0};

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		const auto passContext = currentFrame.commandBuffer->BeginRenderPass(&renderPassInfo);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_context->vkSwapChain->GetExtent().width);
		viewport.height = static_cast<float>(m_context->vkSwapChain->GetExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		passContext->SetViewport(&viewport);

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = m_context->vkSwapChain->GetExtent();
		passContext->SetScissor(&scissor);

		// Bind shader pipeline
		VkPipelineLayout pipelineLayout = m_context->vkPipeline->GetLayout();
		passContext->BindPipeline(m_context->vkPipeline->Handle());

		// Bind global uniforms
		{
			const auto globalUniform = m_globalUniformProvider->Get<GlobalUniformData>(m_currentFrame);
			globalUniform->view = cameraData.view;
			globalUniform->projection = cameraData.projection;
			globalUniform->cameraPosition = cameraData.position;
			globalUniform->lightPosition = directionalLight.position;

			const VkDescriptorSet& globalDescriptorSet = m_context->vkGlobalDescriptorSets[m_currentFrame]->Handle();
			passContext->BindDescriptorSets(pipelineLayout, 0, 1, &globalDescriptorSet);
		}

		for (std::uint32_t i = 0; i < renderList->Size(); i++)
		{
			const auto& [meshGpuData, textureGpuData, transform] = renderList->Get(i);

			// Bind dynamic uniforms
			{
				const auto dynamicUniform = m_dynamicUniformProvider->Get<DynamicUniformData>(m_currentFrame, i);
				dynamicUniform->model = transform;

				VkDescriptorSet		dynamicDescriptorSet = m_context->vkDynamicDescriptorSets[m_currentFrame]->Handle();
				const std::uint32_t offset = m_dynamicUniformProvider->GetOffset(i);
				passContext->BindDescriptorSets(pipelineLayout, 1, 1, &dynamicDescriptorSet, 1, &offset);
			}

			// Bind texture sampler
			{
				const VkDescriptorSet& samplerDescriptorSet = textureGpuData->descriptorSet->Handle();
				passContext->BindDescriptorSets(pipelineLayout, 2, 1, &samplerDescriptorSet);
				// TODO: bind only if different from last frame
			}

			// Draw vertices and indices
			{
				// Bind mesh vertex and index buffers
				const VkBuffer		   vertexBuffers[] = {meshGpuData->vertexBuffer->Handle()};
				constexpr VkDeviceSize offsets[] = {0};
				passContext->BindVertexBuffers(0, 1, vertexBuffers, offsets);
				passContext->BindIndexBuffer(meshGpuData->indexBuffer->Handle(), 0, VK_INDEX_TYPE_UINT32);
			}

			// Draw call
			passContext->DrawIndexed(meshGpuData->indexCount, 1, 0, 0, 0);
		}

		currentFrame.commandBuffer->EndRenderPass();
		currentFrame.commandBuffer->EndRecording();
	}

	void Renderer::CreateCommandBuffers() const
	{
		const auto			device = m_context->vkDevice->Handle();
		const std::uint32_t count = m_context->vkSettings.maxFramesInFlight;

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_context->vkCommandPool->Handle();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		for (std::uint32_t i = 0; i < count; i++)
		{
			VkCommandBuffer commandBuffer;
			if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
				throw std::runtime_error("failed to allocate command buffers!");

			m_context->vkFrames[i].commandBuffer = std::make_unique<VulkanCommandBuffer>(commandBuffer);
		}

		Logger::Log<SUCCESS>("Vulkan command buffers: ok (count: {})", count);
	}

	void Renderer::CreateSyncObjects()
	{
		const std::uint32_t count = m_context->vkSettings.maxFramesInFlight;

		for (std::uint32_t i = 0; i < count; i++)
		{
			auto& frame = m_context->vkFrames[i];
			frame.imageAvailableSemaphore = std::make_unique<VulkanSemaphore>(m_context);
			frame.renderFinishedSemaphore = std::make_unique<VulkanSemaphore>(m_context);
			frame.inFlightFence = std::make_unique<VulkanFence>(m_context);
		}

		Logger::Log<SUCCESS>("Vulkan sync objects: ok (count: {})", count);
	}

	void Renderer::CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding globalUniformLayoutBinding{};
		globalUniformLayoutBinding.binding = 0;
		globalUniformLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		globalUniformLayoutBinding.descriptorCount = 1;
		globalUniformLayoutBinding.pImmutableSamplers = nullptr;
		globalUniformLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding dynamicUniformLayoutBinding{};
		dynamicUniformLayoutBinding.binding = 0;
		dynamicUniformLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		dynamicUniformLayoutBinding.descriptorCount = 1;
		dynamicUniformLayoutBinding.pImmutableSamplers = nullptr;
		dynamicUniformLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 0;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		auto globalBindings = {globalUniformLayoutBinding};
		auto dynamicBindings = {dynamicUniformLayoutBinding};
		auto samplerBindings = {samplerLayoutBinding};

		m_context->vkGlobalDescriptorSetLayout = std::make_unique<VulkanDescriptorSetLayout>(globalBindings, m_context);
		m_context->vkDynamicDescriptorSetLayout = std::make_unique<VulkanDescriptorSetLayout>(dynamicBindings, m_context);
		m_context->vkSamplerDescriptorSetLayout = std::make_unique<VulkanDescriptorSetLayout>(samplerBindings, m_context);

		Logger::Log<SUCCESS>("Vulkan descriptor set layout: ok");
	}

	void Renderer::CreateDescriptorPool()
	{
		const std::uint32_t frames = m_context->vkSettings.maxFramesInFlight;

		VkDescriptorPoolSize globalPoolSize{};
		globalPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		globalPoolSize.descriptorCount = frames; // 1 global UBO par frame

		VkDescriptorPoolSize dynamicPoolSize{};
		dynamicPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		dynamicPoolSize.descriptorCount = frames; // 1 dynamic UBO par frame

		VkDescriptorPoolSize samplerPoolSize{};
		samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerPoolSize.descriptorCount = 1000; // TODO: adjust based on expected number of textures

		std::vector poolSizes = {globalPoolSize, dynamicPoolSize, samplerPoolSize};

		m_context->vkDescriptorPool = std::make_unique<VulkanDescriptorPool>(poolSizes, m_context);

		Logger::Log<SUCCESS>("Vulkan descriptor pool: ok");
	}

	void Renderer::CreateGlobalDescriptorSets()
	{
		m_context->vkGlobalDescriptorSets.resize(m_context->vkSettings.maxFramesInFlight);

		for (size_t i = 0; i < m_context->vkSettings.maxFramesInFlight; i++)
		{
			m_context->vkGlobalDescriptorSets[i] = std::make_unique<VulkanDescriptorSet>(m_context->vkGlobalDescriptorSetLayout, m_context);

			VkDescriptorBufferInfo globalInfo{};
			globalInfo.buffer = m_context->vkFrames[i].globalUniformBuffer->Handle();
			globalInfo.offset = 0;
			globalInfo.range = sizeof(GlobalUniformData);

			const auto& descriptorSet = m_context->vkGlobalDescriptorSets[i]->Handle();

			VkWriteDescriptorSet descriptorWrite{};

			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &globalInfo;

			vkUpdateDescriptorSets(m_context->vkDevice->Handle(), 1, &descriptorWrite, 0, nullptr);
		}

		Logger::Log<SUCCESS>("Vulkan global descriptor sets: ok (count: {})", m_context->vkGlobalDescriptorSets.size());
	}

	void Renderer::CreateDynamicDescriptorSets()
	{
		m_context->vkDynamicDescriptorSets.resize(m_context->vkSettings.maxFramesInFlight);

		for (size_t i = 0; i < m_context->vkSettings.maxFramesInFlight; i++)
		{
			m_context->vkDynamicDescriptorSets[i] = std::make_unique<VulkanDescriptorSet>(m_context->vkDynamicDescriptorSetLayout, m_context);

			VkDescriptorBufferInfo dynamicInfo{};
			dynamicInfo.buffer = m_context->vkFrames[i].dynamicUniformBuffer->Handle();
			dynamicInfo.offset = 0;
			dynamicInfo.range = sizeof(DynamicUniformData);

			const auto& descriptorSet = m_context->vkDynamicDescriptorSets[i]->Handle();

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &dynamicInfo;

			vkUpdateDescriptorSets(m_context->vkDevice->Handle(), 1, &descriptorWrite, 0, nullptr);
		}

		Logger::Log<SUCCESS>("Vulkan dynamic descriptor sets: ok (count: {})", m_context->vkDynamicDescriptorSets.size());
	}

	void Renderer::AddPipeline(const std::function<void(VulkanPipelineBuilder*)>& callback, RenderCache* renderCache)
	{
		VulkanPipelineBuilder builder;
		callback(&builder);

		VulkanPipelineParams params = builder.GetBuildParams();

		std::vector<const ShaderGpuData*> shaderModules;
		shaderModules.reserve(params.shadersInfos.size());

		for (auto& [shaderPath, shaderType] : params.shadersInfos)
		{
			const std::string	 fullShaderPath = m_context->vkSettings.shadersFolderPath + shaderPath;
			const ShaderGpuData* shaderGpuData = renderCache->GetOrCreateShaderGpuData({shaderType, fullShaderPath});
			shaderModules.push_back(shaderGpuData);
		}

		VertexInputDescription vertexInputDescription{};
		vertexInputDescription.bindings = {GetVertexBindingDescription()};
		vertexInputDescription.attributes = GetVertexAttributeDescriptions();

		m_context->vkPipeline = std::make_unique<VulkanPipeline>(params, vertexInputDescription, shaderModules, m_context);
	}

	VkVertexInputBindingDescription Renderer::GetVertexBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(VertexData);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	std::vector<VkVertexInputAttributeDescription> Renderer::GetVertexAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(VertexData, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(VertexData, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(VertexData, uv);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(VertexData, normal);

		return attributeDescriptions;
	}
}
