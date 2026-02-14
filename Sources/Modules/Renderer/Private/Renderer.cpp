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

#include <RenderingApi.hpp>

#include "Renderer/Renderer.hpp"

#include "Module_Data/Graphics/CameraData.hpp"
#include "Module_Data/Graphics/DirectionalLightData.hpp"
#include "Renderer/Data/DynamicUniformData.hpp"
#include "Renderer/Data/GlobalUniformData.hpp"
#include "Renderer/DrawStream.hpp"
#include "Renderer/DynamicUniformProvider.hpp"
#include "Renderer/GlobalUniformProvider.hpp"
#include "Renderer/Passes/ForwardPass.hpp"
#include "Renderer/Passes/ImGuiPass.hpp"
#include "Renderer/Passes/RenderGraph.hpp"
#include "Renderer/RenderList.hpp"
#include "Renderer/RenderMetrics.hpp"

#include <Module_Base/Logger.hpp>
#include <Module_Data/Graphics/VertexData.hpp>
#include <RenderingApi/Old/VertexInputDescription.hpp>

#include <Module_Platform/Manager/WindowManager.hpp>
#include <Module_Platform/Window.hpp>

#include <ResourceManager/MeshGpuData.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/TextureGpuData.hpp>

namespace BetterThanNothing
{
	Renderer::Renderer(VulkanContext* context, ResourceManager* resourceManager) : m_context(context), m_resourceManager(resourceManager)
	{
		const auto [windowWidth, windowHeight] = WindowManager::GetWindow()->GetFramebufferSize();

		BufferManager::Initialize(m_context);

		Logger::Log<INFO>("Buffering mode: {}", m_context->vkSettings.maxFramesInFlight);
		Logger::Log<INFO>("VSync: {}", m_context->vkSettings.bufferingMode == EBufferingMode::Double ? "enabled" : "disabled");

		m_context->instance = Instance::Create({.enableValidation = true});
		m_context->surface = Surface::Create({WindowManager::GetWindow()->Handle(), m_context->instance.get()});
		m_context->device = Device::Create({m_context->instance.get(), m_context->surface.get()});
		m_context->graphicsQueue = m_context->device->CreateGraphicsQueue();
		m_context->presentQueue = m_context->device->CreatePresentQueue();
		m_context->allocator = Allocator::Create({m_context->instance.get(), m_context->device.get()});

		Logger::Log<INFO>("Vendor: {}", m_context->device->GetVendorName());
		Logger::Log<INFO>("Device: {}", m_context->device->GetDeviceName());
		Logger::Log<INFO>("API Version: {}", m_context->device->GetApiVersion());
		Logger::Log<INFO>("Anisotropy: {}x", m_context->device->GetMaxAnisotropy());
		Logger::Log<INFO>("Msaa: {}x", static_cast<std::uint32_t>(m_context->device->GetMaxMsaaSamples()));

		m_context->commandPool = CommandPool::Create({m_context->device.get(), m_context->graphicsQueue.get()});

		m_context->swapChain = SwapChain::Create({
			m_context->surface.get(),
			m_context->device.get(),
			m_context->allocator.get(),
			m_context->commandPool.get(),
			m_context->graphicsQueue.get(),
			windowWidth,
			windowHeight,
			m_context->vkSettings.bufferingMode,
		});

		m_context->frames.resize(m_context->vkSettings.maxFramesInFlight);

		m_globalUniformProvider = std::make_unique<GlobalUniformProvider>(m_context);
		m_dynamicUniformProvider = std::make_unique<DynamicUniformProvider>(m_context, 100000);

		m_renderGraph = std::make_unique<RenderGraph>(m_context);
		m_renderGraph->AddRenderPass<ForwardPass>();
		m_renderGraph->AddRenderPass<ImGuiPass>();

		CreateDescriptorSetLayout();
		CreateDescriptorPool();
		CreateGlobalDescriptorSets();
		CreateDynamicDescriptorSets();

		constexpr std::uint32_t MAX_LIGHT_COUNT = 1000;
		CreatePointLightBuffers(MAX_LIGHT_COUNT);
		CreatePointLightDescriptorSets(MAX_LIGHT_COUNT);

		CreateCommandBuffers();
		CreateSyncObjects();

		Logger::Log<SUCCESS>("Renderer: initialized successfully");
	}

	Renderer::~Renderer()
	{
		m_context->device->WaitIdle();
		BufferManager::Shutdown();
	}

	void Renderer::RenderFrame(const RenderList* renderList, const CameraData& camera)
	{
		if (!m_context->defaultPipeline)
			throw std::runtime_error("No pipeline set for rendering!");

		m_context->frames[m_currentFrame].inFlightFences->Wait();

		const auto device = m_context->device->Handle();
		const auto swapchain = m_context->swapChain->Handle();

		std::uint32_t  imageIndex;
		const VkResult acquireResult =
			vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, m_context->frames[m_currentFrame].imageAvailableSemaphore->Handle(), VK_NULL_HANDLE, &imageIndex);

		if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
		{
			const auto [windowWidth, windowHeight] = WindowManager::GetWindow()->GetFramebufferSize();
			m_context->swapChain->RecreateSwapChain(windowWidth, windowHeight);
			return;
		}

		if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("failed to acquire swap chain image!");

		m_context->frames[m_currentFrame].inFlightFences->Reset();
		m_context->frames[m_currentFrame].commandBuffer->Reset();

		RecordCommandBuffer(renderList, camera, imageIndex);

		const VkSemaphore waitSemaphores[] = {m_context->frames[m_currentFrame].imageAvailableSemaphore->Handle()};

		// We wait for the fragment shader to output the color before we can start writing to the image
		constexpr VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;

		const auto commandBuffer = m_context->frames[m_currentFrame].commandBuffer->Handle();
		submitInfo.pCommandBuffers = &commandBuffer;

		const VkSemaphore signalSemaphores[] = {m_context->frames[m_currentFrame].renderFinishedSemaphore->Handle()};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		auto graphicsQueue = m_context->graphicsQueue->Handle();
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, m_context->frames[m_currentFrame].inFlightFences->Handle()) != VK_SUCCESS)
			throw std::runtime_error("failed to submit draw command buffer!");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		const VkSwapchainKHR swapChains[] = {swapchain};
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

		m_currentFrame = (m_currentFrame + 1) % m_context->frames.size();
	}

	void Renderer::RecordCommandBuffer(const RenderList* renderList, const CameraData& camera, const std ::uint32_t imageIndex) const
	{
		const auto& currentFrame = m_context->frames[m_currentFrame];

		auto cmdContext = currentFrame.commandBuffer->BeginRecording();

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

		DrawStream drawStream;
		for (std::uint32_t i = 0; i < renderList->Size(); i++)
		{
			const auto& [meshId, textureId, transform] = renderList->Get(i);

			const auto meshGpuData = m_resourceManager->GetMeshGpuData(meshId);
			const auto textureGpuData = m_resourceManager->GetTextureGpuData(textureId);

			if (!meshGpuData || !textureGpuData)
				continue;

			const auto dynamicUniform = m_dynamicUniformProvider->Get<DynamicUniformData>(m_currentFrame, i);
			dynamicUniform->model = transform;

			drawStream.Draw({
				.pipeline = m_context->defaultPipeline->Handle(),
				.pipelineLayout = m_context->defaultPipeline->GetLayout(),
				.vertexBuffer = meshGpuData->vertexBuffer->Handle(),
				.indexBuffer = meshGpuData->indexBuffer->Handle(),
				.indexCount = meshGpuData->indexCount,
				.globalDescriptorSet = m_context->vkGlobalDescriptorSets[m_currentFrame]->Handle(),
				.dynamicDescriptorSet = m_context->vkDynamicDescriptorSets[m_currentFrame]->Handle(),
				.dynamicOffset = m_dynamicUniformProvider->GetOffset(i),
				.samplerDescriptorSet = textureGpuData->descriptorSet->Handle(),
				.pointLightDescriptorSet = m_context->vkPointLightDescriptorSets[m_currentFrame]->Handle(),
			});

			RenderMetrics::AddDrawCall(meshGpuData->vertexCount);
		}

		RenderPassContext renderPassContext{};
		renderPassContext.commandBuffer = &cmdContext;
		renderPassContext.imageIndex = imageIndex;
		renderPassContext.currentFrame = m_currentFrame;

		m_renderGraph->Execute(renderPassContext, drawStream);

		currentFrame.commandBuffer->EndRecording();
	}

	void Renderer::DrawPointLights(const RenderList* renderList) const
	{
		const std::uint32_t count = renderList->GetPointLightCount();

		if (count == 0)
			return;

		void* dstData = m_context->frames[m_currentFrame].pointLightBuffer->GetMappedData();
		std::memcpy(dstData, renderList->GetPointLights().data(), count * sizeof(PointLightData));
	}

	void Renderer::CreateCommandBuffers() const
	{
		const auto			device = m_context->device->Handle();
		const std::uint32_t count = m_context->vkSettings.maxFramesInFlight;

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_context->commandPool->Handle();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		for (std::uint32_t i = 0; i < count; i++)
		{
			VkCommandBuffer commandBuffer;
			if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
				throw std::runtime_error("failed to allocate command buffers!");

			m_context->frames[i].commandBuffer = std::make_unique<CommandBuffer>(commandBuffer);
		}

		Logger::Log<SUCCESS>("Vulkan command buffers: ok (count: {})", count);
	}

	void Renderer::CreateSyncObjects()
	{
		const std::uint32_t count = m_context->vkSettings.maxFramesInFlight;

		for (std::uint32_t i = 0; i < count; i++)
		{
			auto& frame = m_context->frames[i];
			frame.imageAvailableSemaphore = Semaphore::Create({m_context->device.get()});
			frame.renderFinishedSemaphore = Semaphore::Create({m_context->device.get()});
			frame.inFlightFences = Fence::Create({m_context->device.get(), true});
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

		VkDescriptorSetLayoutBinding pointLightLayoutBinding{};
		pointLightLayoutBinding.binding = 0;
		pointLightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		pointLightLayoutBinding.descriptorCount = 1;
		pointLightLayoutBinding.pImmutableSamplers = nullptr;
		pointLightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		auto globalBindings = {globalUniformLayoutBinding};
		auto dynamicBindings = {dynamicUniformLayoutBinding};
		auto samplerBindings = {samplerLayoutBinding};
		auto pointLightBinding = {pointLightLayoutBinding};

		m_context->vkGlobalDescriptorSetLayout = std::make_unique<VulkanDescriptorSetLayout>(globalBindings, m_context);
		m_context->vkDynamicDescriptorSetLayout = std::make_unique<VulkanDescriptorSetLayout>(dynamicBindings, m_context);
		m_context->vkSamplerDescriptorSetLayout = std::make_unique<VulkanDescriptorSetLayout>(samplerBindings, m_context);
		m_context->vkPointLightDescriptorLayout = std::make_unique<VulkanDescriptorSetLayout>(pointLightBinding, m_context);

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

		VkDescriptorPoolSize pointLightPoolSize{};
		pointLightPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		pointLightPoolSize.descriptorCount = frames;

		std::vector poolSizes = {globalPoolSize, dynamicPoolSize, samplerPoolSize, pointLightPoolSize};

		m_context->vkDescriptorPool = std::make_unique<DescriptorPool>(poolSizes, m_context);

		Logger::Log<SUCCESS>("Vulkan descriptor pool: ok");
	}

	void Renderer::CreateGlobalDescriptorSets()
	{
		m_context->vkGlobalDescriptorSets.resize(m_context->vkSettings.maxFramesInFlight);

		for (size_t i = 0; i < m_context->vkSettings.maxFramesInFlight; i++)
		{
			m_context->vkGlobalDescriptorSets[i] = std::make_unique<VulkanDescriptorSet>(m_context->vkGlobalDescriptorSetLayout, m_context);

			VkDescriptorBufferInfo globalInfo{};
			globalInfo.buffer = m_context->frames[i].globalUniformBuffer->Handle();
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

	void Renderer::CreateDynamicDescriptorSets()
	{
		m_context->vkDynamicDescriptorSets.resize(m_context->vkSettings.maxFramesInFlight);

		for (size_t i = 0; i < m_context->vkSettings.maxFramesInFlight; i++)
		{
			m_context->vkDynamicDescriptorSets[i] = std::make_unique<VulkanDescriptorSet>(m_context->vkDynamicDescriptorSetLayout, m_context);

			VkDescriptorBufferInfo dynamicInfo{};
			dynamicInfo.buffer = m_context->frames[i].dynamicUniformBuffer->Handle();
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

			vkUpdateDescriptorSets(m_context->device->Handle(), 1, &descriptorWrite, 0, nullptr);
		}

		Logger::Log<SUCCESS>("Vulkan dynamic descriptor sets: ok (count: {})", m_context->vkDynamicDescriptorSets.size());
	}

	void Renderer::CreatePointLightBuffers(std::uint32_t maxLights) const
	{
		const std::uint32_t count = m_context->vkSettings.maxFramesInFlight;
		const std::uint32_t bufferSize = maxLights * sizeof(PointLightData);

		for (std::uint32_t i = 0; i < count; i++)
		{
			m_context->frames[i].pointLightBuffer = BufferManager::CreateStorageBuffer(bufferSize);
		}

		Logger::Log<SUCCESS>("Point light buffers: ok (count: {})", count);
	}

	void Renderer::CreatePointLightDescriptorSets(std::uint32_t maxLights)
	{
		m_context->vkPointLightDescriptorSets.resize(m_context->vkSettings.maxFramesInFlight);

		const VkDeviceSize lightBufferSize = sizeof(PointLightData) * maxLights;

		for (size_t i = 0; i < m_context->vkSettings.maxFramesInFlight; i++)
		{
			m_context->vkPointLightDescriptorSets[i] = std::make_unique<VulkanDescriptorSet>(m_context->vkPointLightDescriptorLayout, m_context);

			VkDescriptorBufferInfo dynamicInfo{};
			dynamicInfo.buffer = m_context->frames[i].pointLightBuffer->Handle();
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

	void Renderer::AddPipeline(const std::function<void(VulkanPipelineBuilder*)>& callback)
	{
		VulkanPipelineBuilder builder;
		callback(&builder);

		VulkanPipelineParams params = builder.GetBuildParams();

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

		m_context->defaultPipeline = std::make_unique<Pipeline>(params, vertexInputDescription, shaderModules, m_context);
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
