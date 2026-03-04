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

#include "Renderer/Renderer.hpp"
#include "Module_Data/Graphics/CameraData.hpp"
#include "Module_Data/Graphics/DirectionalLightData.hpp"
#include "Module_Data/Graphics/DynamicUniformData.hpp"
#include "Module_Data/Graphics/GlobalUniformData.hpp"
#include "Renderer/GlobalUniformProvider.hpp"
#include "Renderer/Passes/ForwardPass.hpp"
#include "Renderer/Passes/ImGuiPass.hpp"
#include "Renderer/Passes/RenderGraph.hpp"
#include "Renderer/RenderList.hpp"
#include "Renderer/RenderMetrics.hpp"
#include "RenderingApi/Resources/DescriptorSet.hpp"
#include "RenderingDevice/DrawStream.hpp"
#include "ResourceManager/ResourceManager_Types.hpp"

#include <Module_Base/Logger.hpp>
#include <Module_Data/Graphics/VertexData.hpp>
#include <Module_Platform/Manager/WindowManager.hpp>
#include <Module_Platform/Window.hpp>
#include <RenderingApi.hpp>
#include <RenderingApi/Deprecated/VertexInputDescription.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <bits/stl_queue.h>
#include <cstring>

namespace BetterThanNothing
{
	Renderer::Renderer(VulkanContext* context, ResourceManager* resourceManager, RenderingDevice* renderingDevice)
		: m_context(context), m_resourceManager(resourceManager), m_renderingDevice(renderingDevice)
	{
		m_context->settings.maxFramesInFlight = m_renderingDevice->GetMaxFramesInFlight();

		m_context->instance = m_renderingDevice->GetInstance();
		m_context->surface = m_renderingDevice->GetSurface();
		m_context->device = m_renderingDevice->GetDevice();
		m_context->graphicsQueue = m_renderingDevice->GetGraphicsQueue();
		m_context->presentQueue = m_renderingDevice->GetPresentQueue();
		m_context->allocator = m_renderingDevice->GetAllocator();
		m_context->commandPool = m_renderingDevice->GetCommandPool();
		m_context->swapChain = m_renderingDevice->GetSwapChain();

		m_globalUniformProvider = std::make_unique<GlobalUniformProvider>(m_context);

		m_renderGraph = std::make_unique<RenderGraph>(m_context, m_renderingDevice);
		m_renderGraph->AddRenderPass<ForwardPass>();
		m_renderGraph->AddRenderPass<ImGuiPass>();

		m_context->descriptorPool = m_renderingDevice->GetDescriptorPool();

		CreateGlobalDescriptorSets();

		constexpr uint32_t MAX_LIGHT_COUNT = 1000;
		CreatePointLightBuffers(MAX_LIGHT_COUNT);
		CreatePointLightDescriptorSets(MAX_LIGHT_COUNT);

		CreateCommandBuffers();
		CreateSyncObjects();

		AddPipeline([&](RenderingApi::PipelineBuilder* builder) {
			builder->SetPath("../../../Assets/Shaders/");
			builder->SetName("main");
			builder->AddVertexShader("main/main.vert");
			builder->AddFragmentShader("main/main.frag");
			builder->AddDescriptorSetLayout(m_renderingDevice->GetDescriptorSetLayout(DescriptorSetLayoutType::GlobalUniforms)->Handle());
			builder->AddDescriptorSetLayout(m_renderingDevice->GetDescriptorSetLayout(DescriptorSetLayoutType::Textures)->Handle());
			builder->AddDescriptorSetLayout(m_renderingDevice->GetDescriptorSetLayout(DescriptorSetLayoutType::PointLights)->Handle());
		});

		m_drawStream = std::make_unique<DrawStream>();

		Logger::Log<SUCCESS>("Renderer: initialized successfully");
	}

	Renderer::~Renderer()
	{
		m_context->device->WaitIdle();
	}

	void Renderer::RenderFrame(const RenderList* renderList, const CameraData& camera)
	{
		if (!m_context->defaultPipeline)
			throw std::runtime_error("No pipeline set for rendering!");

		m_context->inFlightFences[m_currentFrame]->Wait();

		m_renderingDevice->FlushDescriptorSetUpdateRequests();

		const auto acquireResult = m_context->swapChain->AcquireNextImage(m_context->imageAvailableSemaphore[m_currentFrame]->Handle());

		if (!acquireResult.has_value())
		{
			const auto [windowWidth, windowHeight] = WindowManager::GetWindow()->GetFramebufferSize();
			m_context->swapChain->RecreateSwapChain(windowWidth, windowHeight);
			return;
		}

		const uint32_t imageIndex = acquireResult.value();

		m_context->inFlightFences[m_currentFrame]->Reset();
		m_context->commandBuffer[m_currentFrame]->Reset();

		RecordCommandBuffer(renderList, camera, imageIndex);

		const auto					   commandBuffer = m_context->commandBuffer[m_currentFrame]->Handle();
		const VkSemaphore			   waitSemaphores[] = {m_context->imageAvailableSemaphore[m_currentFrame]->Handle()};
		constexpr VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		const VkSemaphore			   signalSemaphores[] = {m_context->swapChain->GetImageFinishedSemaphore(imageIndex)};

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		auto graphicsQueue = m_context->graphicsQueue->Handle();
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, m_context->inFlightFences[m_currentFrame]->Handle()) != VK_SUCCESS)
			throw std::runtime_error("failed to submit draw command buffer!");

		const VkSwapchainKHR swapChains[] = {m_context->swapChain->Handle()};

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		const VkResult presentResult = vkQueuePresentKHR(m_context->presentQueue->Handle(), &presentInfo);

		auto& window = WindowManager::GetWindow();
		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || window->IsResized())
		{
			const auto [windowWidth, windowHeight] = window->GetFramebufferSize();
			window->SetResized(false);
			m_context->swapChain->RecreateSwapChain(windowWidth, windowHeight);
		}
		else if (presentResult != VK_SUCCESS)
			throw std::runtime_error("failed to present swap chain image!");

		m_currentFrame = (m_currentFrame + 1) % m_renderingDevice->GetMaxFramesInFlight();
	}

	void Renderer::RecordCommandBuffer(const RenderList* renderList, const CameraData& camera, const std ::uint32_t imageIndex) const
	{
		m_context->commandBuffer[m_currentFrame]->BeginRecording();

		auto directionalLight = renderList->GetDirectionalLight();

		const auto globalUniform = m_globalUniformProvider->Get<GlobalUniformData>(m_currentFrame);
		globalUniform->view = camera.view;
		globalUniform->projection = camera.projection;
		globalUniform->cameraPosition = camera.position;
		globalUniform->cameraExposure = camera.exposure;
		globalUniform->lightDirection = directionalLight.revDirection;
		globalUniform->lightColor = directionalLight.color;
		globalUniform->lightIntensity = directionalLight.intensity;
		globalUniform->pointLightCount = renderList->GetPointLightCount();

		DrawPointLights(renderList);

		RenderMetrics::SetCameraCoordinates(camera.position.GetX(), camera.position.GetY(), camera.position.GetZ());

		m_drawStream->Reset(renderList->Size());

		for (uint32_t i = 0; i < renderList->Size(); i++)
		{
			const auto& [meshId, textureId, transform] = renderList->Get(i);

			const auto meshData = m_resourceManager->GetMeshData(meshId);
			const auto textureData = m_resourceManager->GetTextureData(textureId);

			if (!meshData || !textureData)
				continue;

			auto meshPosition = Vector3(transform.Value()[3][0], transform.Value()[3][1], transform.Value()[3][2]);

			m_drawStream->Draw(
				{.pipeline = m_context->defaultPipeline->Handle(),
				 .pipelineLayout = m_context->defaultPipeline->GetLayout(),
				 .vertexBuffer = meshData->vertexBuffer,
				 .indexBuffer = meshData->indexBuffer,
				 .indexCount = meshData->indexCount,
				 .globalDescriptorSet = m_context->vkGlobalDescriptorSets[m_currentFrame]->Handle(),
				 .samplerDescriptorSet = textureData->descriptorSetHandle,
				 .pointLightDescriptorSet = m_context->vkPointLightDescriptorSets[m_currentFrame]->Handle(),
				 .transform = transform,
				 .distanceToCamera = Vector3::Distance(camera.position, meshPosition)}
			);

			RenderMetrics::AddDrawCall(meshData->vertexCount);
		}

		RenderPassContext renderPassContext{};
		renderPassContext.commandBuffer = m_context->commandBuffer[m_currentFrame].get();
		renderPassContext.imageIndex = imageIndex;
		renderPassContext.currentFrame = m_currentFrame;

		m_renderGraph->Execute(renderPassContext, m_drawStream->Build());

		m_context->commandBuffer[m_currentFrame]->EndRecording();
	}

	void Renderer::DrawPointLights(const RenderList* renderList) const
	{
		const uint32_t count = renderList->GetPointLightCount();

		if (count == 0)
			return;

		void* dstData = m_context->pointLightBuffer[m_currentFrame]->GetMappedData();
		std::memcpy(dstData, renderList->GetPointLights().data(), count * sizeof(PointLightData));
	}

	void Renderer::CreateCommandBuffers() const
	{
		const uint32_t count = m_context->settings.maxFramesInFlight;

		m_context->commandBuffer.resize(count);
		for (uint32_t i = 0; i < count; i++)
			m_context->commandBuffer[i] = RenderingApi::CommandBuffer::Create({m_context->device, m_context->commandPool});

		Logger::Log<SUCCESS>("Vulkan command buffers: ok (count: {})", count);
	}

	void Renderer::CreateSyncObjects() const
	{
		const uint32_t frameCount = m_context->settings.maxFramesInFlight;

		m_context->imageAvailableSemaphore.resize(frameCount);
		m_context->inFlightFences.resize(frameCount);

		for (uint32_t i = 0; i < frameCount; i++)
		{
			m_context->imageAvailableSemaphore[i] = RenderingApi::Semaphore::Create({m_context->device});
			m_context->inFlightFences[i] = RenderingApi::Fence::Create({m_context->device, true});
		}

		Logger::Log<SUCCESS>("Vulkan sync objects: ok (frames: {})", frameCount);
	}

	void Renderer::CreateGlobalDescriptorSets()
	{
		const auto* globalDescriptorSetLayout = m_renderingDevice->GetDescriptorSetLayout(DescriptorSetLayoutType::GlobalUniforms);

		m_context->vkGlobalDescriptorSets.resize(m_context->settings.maxFramesInFlight);

		for (size_t i = 0; i < m_context->settings.maxFramesInFlight; i++)
		{
			m_context->vkGlobalDescriptorSets[i] = std::make_unique<VulkanDescriptorSet>(globalDescriptorSetLayout, m_context);

			VkDescriptorBufferInfo globalInfo{};
			globalInfo.buffer = m_context->globalUniformBuffer[i]->Handle();
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

			vkUpdateDescriptorSets(m_context->device->Handle(), 1, &descriptorWrite, 0, nullptr);
		}

		Logger::Log<SUCCESS>("Vulkan global descriptor sets: ok (count: {})", m_context->vkGlobalDescriptorSets.size());
	}

	void Renderer::CreatePointLightBuffers(uint32_t maxLights) const
	{
		const uint32_t count = m_context->settings.maxFramesInFlight;
		const uint32_t bufferSize = maxLights * sizeof(PointLightData);

		m_context->pointLightBuffer.resize(count);
		for (uint32_t i = 0; i < count; i++)
			m_context->pointLightBuffer[i] = std::make_unique<VulkanStorageBuffer>(m_context, bufferSize);

		Logger::Log<SUCCESS>("Point light buffers: ok (count: {})", count);
	}

	void Renderer::CreatePointLightDescriptorSets(uint32_t maxLights)
	{
		const auto* pointLightDescriptorSetLayout = m_renderingDevice->GetDescriptorSetLayout(DescriptorSetLayoutType::PointLights);

		m_context->vkPointLightDescriptorSets.resize(m_context->settings.maxFramesInFlight);

		const VkDeviceSize lightBufferSize = sizeof(PointLightData) * maxLights;

		for (size_t i = 0; i < m_context->settings.maxFramesInFlight; i++)
		{
			m_context->vkPointLightDescriptorSets[i] = std::make_unique<VulkanDescriptorSet>(pointLightDescriptorSetLayout, m_context);

			VkDescriptorBufferInfo dynamicInfo{};
			dynamicInfo.buffer = m_context->pointLightBuffer[i]->Handle();
			dynamicInfo.offset = 0;
			dynamicInfo.range = lightBufferSize;

			const auto& descriptorSet = m_context->vkPointLightDescriptorSets[i]->Handle();

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &dynamicInfo;

			vkUpdateDescriptorSets(m_context->device->Handle(), 1, &descriptorWrite, 0, nullptr);
		}

		Logger::Log<SUCCESS>("Point light descriptor sets: ok (count: {})", m_context->vkPointLightDescriptorSets.size());
	}

	void Renderer::AddPipeline(const std::function<void(RenderingApi::PipelineBuilder*)>& callback)
	{
		RenderingApi::PipelineBuilder builder;
		callback(&builder);

		RenderingApi::PipelineParams params = builder.GetBuildParams();

		std::vector<const ShaderGpuData*> shaderModules;
		shaderModules.reserve(params.shadersInfos.size());

		for (auto& [shaderPath, shaderType] : params.shadersInfos)
		{
			const ResourceId	 shaderId = m_resourceManager->LoadShader(params.path + shaderPath, shaderType);
			const ShaderGpuData* shaderGpuData = m_resourceManager->GetShader(shaderId);
			shaderModules.push_back(shaderGpuData);
		}

		VertexInputDescription vertexInputDescription{};
		vertexInputDescription.bindings = {GetVertexBindingDescription()};
		vertexInputDescription.attributes = GetVertexAttributeDescriptions();

		m_context->defaultPipeline = std::make_unique<RenderingApi::Pipeline>(params, vertexInputDescription, shaderModules, m_context);
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
