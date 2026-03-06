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
#include "../Public/Renderer/ImGuiPass.hpp"
#include "Module_Data/Graphics/CameraData.hpp"
#include "Module_Data/Graphics/DirectionalLightData.hpp"
#include "Module_Data/Graphics/DynamicUniformData.hpp"
#include "Module_Data/Graphics/GlobalUniformData.hpp"
#include "Renderer/GlobalUniformProvider.hpp"
#include "Renderer/RenderList.hpp"
#include "Renderer/RenderMetrics.hpp"
#include "RenderingApi/Resources/Buffer.hpp"
#include "RenderingApi/Resources/DescriptorSet.hpp"
#include "RenderingDevice/DrawStream.hpp"
#include "ResourceManager/ResourceManager_Types.hpp"
#include <Module_Base/Logger.hpp>
#include <Module_Data/Graphics/VertexData.hpp>
#include <Module_Platform/Manager/WindowManager.hpp>
#include <Module_Platform/Window.hpp>
#include <RenderingApi.hpp>
#include <RenderingApi/Data/VertexInputDescription.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <cstring>
#include <optional>

namespace BetterThanNothing
{
	Renderer::Renderer(VulkanContext* context, ResourceManager* resourceManager, RenderingDevice* renderingDevice)
		: m_context(context), m_resourceManager(resourceManager), m_renderingDevice(renderingDevice)
	{
		m_context->settings.maxFramesInFlight = m_renderingDevice->GetMaxFramesInFlight();

		m_context->instance = m_renderingDevice->GetInstance();
		m_context->device = m_renderingDevice->GetDevice();
		m_context->graphicsQueue = m_renderingDevice->GetGraphicsQueue();
		m_context->allocator = m_renderingDevice->GetAllocator();
		m_context->swapChain = m_renderingDevice->GetSwapChain();

		m_globalUniformProvider = std::make_unique<GlobalUniformProvider>(m_context);

		m_imGuiPass = std::make_unique<ImGuiPass>(m_renderingDevice);

		m_context->descriptorPool = m_renderingDevice->GetDescriptorPool();

		CreateGlobalDescriptorSets();

		constexpr uint32_t MAX_LIGHT_COUNT = 1000;
		CreatePointLightBuffers(MAX_LIGHT_COUNT);
		CreatePointLightDescriptorSets(MAX_LIGHT_COUNT);

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

	}

	void Renderer::RenderFrame(const RenderList* renderList, const CameraData& camera) const
	{
		if (!m_context->defaultPipeline)
			throw std::runtime_error("No pipeline set for rendering!");

		const auto imageIndexOpt = m_renderingDevice->BeginFrame();
		if (!imageIndexOpt)
			return;

		const uint32_t imageIndex = imageIndexOpt.value();

		RecordCommandBuffer(renderList, camera, imageIndex);

		m_renderingDevice->EndFrame(imageIndex);
	}

	void Renderer::RecordCommandBuffer(const RenderList* renderList, const CameraData& camera, const uint32_t imageIndex) const
	{
		const uint32_t frameIndex = m_renderingDevice->GetCurrentFrameIndex();
		const auto directionalLight = renderList->GetDirectionalLight();

		const auto globalUniform = m_globalUniformProvider->Get<GlobalUniformData>(frameIndex);
		globalUniform->view = camera.view;
		globalUniform->projection = camera.projection;
		globalUniform->cameraPosition = camera.position;
		globalUniform->cameraExposure = camera.exposure;
		globalUniform->lightDirection = directionalLight.revDirection;
		globalUniform->lightColor = directionalLight.color;
		globalUniform->lightIntensity = directionalLight.intensity;
		globalUniform->pointLightCount = renderList->GetPointLightCount();

		DrawPointLights(renderList, frameIndex);

		RenderMetrics::SetCameraCoordinates(camera.position.GetX(), camera.position.GetY(), camera.position.GetZ());

		m_drawStream->Reset(renderList->Size());

		for (uint32_t i = 0; i < renderList->Size(); i++)
		{
			const auto& [meshId, textureId, transform] = renderList->Get(i);

			const auto meshData = m_resourceManager->GetMeshData(meshId);
			const auto textureData = m_resourceManager->GetTextureData(textureId);

			if (!meshData || !textureData)
				continue;

			// clang-format off
			m_drawStream->Draw({
				.pipeline = m_context->defaultPipeline->Handle(),
				.pipelineLayout = m_context->defaultPipeline->GetLayout(),
				.vertexBuffer = meshData->vertexBuffer,
				.indexBuffer = meshData->indexBuffer,
				.indexCount = meshData->indexCount,
				.globalDescriptorSet = m_context->vkGlobalDescriptorSets[frameIndex]->Handle(),
				.samplerDescriptorSet = textureData->descriptorSetHandle,
				.pointLightDescriptorSet = m_context->vkPointLightDescriptorSets[frameIndex]->Handle(),
				.transform = transform,
				.distanceToCamera = Vector3::Distance(
					camera.position,
					Vector3(transform.Value()[3][0], transform.Value()[3][1], transform.Value()[3][2])
				)
			});
			// clang-format on

			RenderMetrics::AddDrawCall(meshData->vertexCount);
		}

		const auto drawStream = m_drawStream->Build();

		ExecuteForwardPass(imageIndex, drawStream);
		m_imGuiPass->Execute(imageIndex);
	}

	void Renderer::DrawPointLights(const RenderList* renderList, const uint32_t frameIndex) const
	{
		const uint32_t count = renderList->GetPointLightCount();

		if (count == 0)
			return;

		void* dstData = m_context->pointLightBuffer[frameIndex]->GetMappedData();
		std::memcpy(dstData, renderList->GetPointLights().data(), count * sizeof(PointLightData));
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

	void Renderer::ExecuteForwardPass(uint32_t imageIndex, const DrawStreamView& drawStream) const
	{
		const auto commandBuffer = m_renderingDevice->GetCurrentCommandBuffer();

		{
			std::array<VkImageMemoryBarrier, 3> barriers{};

			barriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barriers[0].srcAccessMask = 0;
			barriers[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barriers[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barriers[0].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[0].image = m_context->swapChain->GetImage(imageIndex);
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

			commandBuffer->PipelineBarrier(
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
		colorAttachment.resolveImageView = m_context->swapChain->GetImageView(imageIndex);
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
				Handle lastSamplerHandle = {};

				bool	   boundGlobalDescriptor = false;
				bool	   boundPointLightDescriptor = false;

				for (const DrawPacket& drawPacket : drawStream)
				{
					if (lastPipeline != drawPacket.pipeline)
					{
						commandBuffer->BindPipeline(drawPacket.pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
						lastPipeline = drawPacket.pipeline;
					}

					if (!boundGlobalDescriptor)
					{
						commandBuffer->BindDescriptorSets(drawPacket.pipelineLayout, 0, 1, &drawPacket.globalDescriptorSet, 0, nullptr);
						boundGlobalDescriptor = true;
					}

					if (!boundPointLightDescriptor)
					{
						commandBuffer->BindDescriptorSets(drawPacket.pipelineLayout, 2, 1, &drawPacket.pointLightDescriptorSet, 0, nullptr);
						boundPointLightDescriptor = true;
					}

					if (lastSamplerHandle != drawPacket.samplerDescriptorSet)
					{
						const auto samplerDescriptorSet = m_renderingDevice->GetDescriptorSet(drawPacket.samplerDescriptorSet);
						commandBuffer->BindDescriptorSets(drawPacket.pipelineLayout, 1, 1, &samplerDescriptorSet->value, 0, nullptr);
						lastSamplerHandle = drawPacket.samplerDescriptorSet;
					}

					const DynamicUniformData dynamicUniforms = {drawPacket.transform};
					commandBuffer->PushConstants(drawPacket.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(DynamicUniformData), &dynamicUniforms);

					const auto vertexBuffer = m_renderingDevice->GetBuffer(drawPacket.vertexBuffer);
					const auto indexBuffer = m_renderingDevice->GetBuffer(drawPacket.indexBuffer);

					const VkBuffer		   vertexBuffers[] = {vertexBuffer->value};
					constexpr VkDeviceSize offsets[] = {0};
					commandBuffer->BindVertexBuffers(0, 1, vertexBuffers, offsets);
					commandBuffer->BindIndexBuffer(indexBuffer->value, 0, VK_INDEX_TYPE_UINT32);

					commandBuffer->DrawIndexed(drawPacket.indexCount, 1, 0, 0, 0);
				}
			},
			renderingInfo
		);

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
