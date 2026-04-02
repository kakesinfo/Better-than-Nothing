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

#include "RenderingApi/Core/PipelineLayout.hpp"
#include "RenderingDevice/Blueprint/PipelineLayoutBlueprint.hpp"
#include "RenderingDevice/VertexLayout.hpp"
#include "ResourceManager/PipelineBlueprint.hpp"

#include <Common/Logger.hpp>
#include <Common/Metrics.hpp>
#include <Data/Graphics/CameraData.hpp>
#include <Data/Graphics/DirectionalLightData.hpp>
#include <Data/Graphics/DynamicUniformData.hpp>
#include <Data/Graphics/GlobalUniformData.hpp>
#include <Data/Graphics/VertexData.hpp>
#include <Editor/Editor.hpp>
#include <RenderingApi.hpp>
#include <RenderingApi/Data/VertexInputDescription.hpp>
#include <RenderingApi/Resources/Buffer.hpp>
#include <RenderingApi/Resources/DescriptorSet.hpp>
#include <RenderingDevice/DrawStream.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>
#include <Scene/Camera.hpp>
#include <Scene/Scene.hpp>
#include <Scene/SceneObject.hpp>
#include <Scene/SceneObject_Components.hpp>
#include <optional>

#include <iostream>

namespace BetterThanNothing
{
	constexpr uint32_t MAX_LIGHT_COUNT = 1000;

	Renderer::Renderer(RenderingDevice* renderingDevice, ResourceManager* resourceManager, Editor* editor)
		: m_renderingDevice(renderingDevice), m_resourceManager(resourceManager), m_editor(editor)
	{
		m_renderingDevice->CreateDescriptorSetLayout("GlobalUniforms", VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		m_renderingDevice->CreateDescriptorSetLayout("Textures", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		m_renderingDevice->CreateDescriptorSetLayout("PointLights", VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);

		m_renderingDevice->CreatePipelineLayout("Forward", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("Textures");
			blueprint->AddDescriptorSetLayout("PointLights");
			blueprint->AddPushConstant<DynamicUniformData>(EShaderType::Vertex);
		});

		CreateGlobalUniformBuffers();
		CreateGlobalUniformDescriptorSets();
		CreatePointLightBuffers();
		CreatePointLightDescriptorSets();

		const VertexLayout vertexLayout = { VertexType::Float3, VertexType::Float3, VertexType::Float2, VertexType::Float3 };

		const auto vertShader = m_resourceManager->LoadShader("../../../Shaders/main/main.vert");
		const auto fragShader = m_resourceManager->LoadShader("../../../Shaders/main/main.frag");

		m_resourceManager->CreatePipeline("Forward::Opaque", [&](PipelineBlueprint* blueprint) {
			blueprint->AddShader(vertShader);
			blueprint->AddShader(fragShader);
			blueprint->SetPipelineLayout("Forward");
			blueprint->SetVertexLayout(vertexLayout);
		});

		AddPipeline([&](RenderingApi::PipelineBuilder* builder) {
			builder->SetPath("../../../Shaders/");
			builder->SetName("main");
			builder->AddVertexShader("main/main.vert");
			builder->AddFragmentShader("main/main.frag");
			builder->AddDescriptorSetLayout(m_renderingDevice->GetDescriptorSetLayout("GlobalUniforms")->Handle());
			builder->AddDescriptorSetLayout(m_renderingDevice->GetDescriptorSetLayout("Textures")->Handle());
			builder->AddDescriptorSetLayout(m_renderingDevice->GetDescriptorSetLayout("PointLights")->Handle());
		});

		m_drawStream = std::make_unique<DrawStream>();

		Logger::Log<SUCCESS>("Renderer: initialized successfully");
	}

	Renderer::~Renderer()
	{
		if (m_renderingDevice)
			m_renderingDevice->GetDevice()->WaitIdle();
	}

	std::unique_ptr<Renderer> Renderer::Create(RenderingDevice* renderingDevice, ResourceManager* resourceManager, Editor* debugUI)
	{
		return std::make_unique<Renderer>(renderingDevice, resourceManager, debugUI);
	}

	void Renderer::RenderFrame(Scene* scene) const
	{
		if (!m_defaultPipeline)
			throw std::runtime_error("No pipeline set for rendering!");

		const auto imageIndexOpt = m_renderingDevice->BeginFrame();
		if (!imageIndexOpt)
			return;

		const uint32_t imageIndex = imageIndexOpt.value();

		RecordCommandBuffer(scene, imageIndex);

		m_renderingDevice->EndFrame(imageIndex);
	}

	struct RenderObject
	{
		ResourceId meshId;
		ResourceId textureId;
		Matrix4	   localMatrix;
	};

	static std::vector<RenderObject> FlattenScene(const Scene* scene)
	{
		const auto modelView = scene->GetView<TransformComponent, ModelComponent>();

		std::vector<RenderObject> objects;
		for (auto [entity, transform, model] : modelView.each())
		{
			for (const auto& sceneObj : model.children)
			{
				if (!sceneObj.HasComponent<MeshComponent>())
					continue;

				const auto& meshComp = sceneObj.GetComponent<MeshComponent>();

				objects.emplace_back(RenderObject{.meshId = meshComp.meshId, .textureId = meshComp.textureId, .localMatrix = transform.localTransform});
			}
		}

		return objects;
	}

	void Renderer::RecordCommandBuffer(Scene* scene, const uint32_t imageIndex) const
	{
		const uint32_t frameIndex = m_renderingDevice->GetCurrentFrameIndex();
		const float	   aspectRatio = m_renderingDevice->GetAspectRatio();

		const auto& directionalLight = scene->GetDirectionalLight();
		const auto& camera = scene->GetCamera();

		const auto globalUniform = GetGlobalUniformBuffer(frameIndex);
		globalUniform->view = camera->GetViewMatrix();
		globalUniform->projection = camera->GetProjectionMatrix(aspectRatio);
		globalUniform->cameraPosition = camera->GetPosition();
		globalUniform->cameraExposure = 0.5f;
		globalUniform->lightDirection = directionalLight.revDirection;
		globalUniform->lightColor = directionalLight.color;
		globalUniform->lightIntensity = directionalLight.intensity;
		globalUniform->pointLightCount = scene->GetPointLightCount();

		DrawPointLights(scene, frameIndex);

		Metrics::SetCameraCoordinates(camera->GetPosition().GetX(), camera->GetPosition().GetY(), camera->GetPosition().GetZ());

		const auto flattenedScene = FlattenScene(scene);

		m_drawStream->Reset(flattenedScene.size());

		for (const auto& [meshId, textureId, localMatrix] : flattenedScene)
		{
			if (!meshId || !textureId)
				continue;

			const auto meshData = m_resourceManager->GetMeshData(meshId);
			const auto textureData = m_resourceManager->GetTextureData(textureId);

			if (!meshData || !textureData)
				continue;

			const float distance =
				Vector3::Distance2(camera->GetPosition(), Vector3(localMatrix.Value()[3][0], localMatrix.Value()[3][1], localMatrix.Value()[3][2]));

			// clang-format off
			m_drawStream->Draw({
				.pipeline = m_defaultPipeline->Handle(),
				.pipelineLayout = m_defaultPipeline->GetLayout(),
				.vertexBuffer = meshData->vertexBuffer,
				.indexBuffer = meshData->indexBuffer,
				.indexCount = meshData->indexCount,
				.globalDescriptorSet = m_renderingDevice->GetDescriptorSet(m_globalDescriptorSets[frameIndex])->value,
				.samplerDescriptorSet = textureData->descriptorSetHandle,
				.pointLightDescriptorSet = m_renderingDevice->GetDescriptorSet(m_pointLightDescriptorSets[frameIndex])->value,
				.transform = localMatrix,
				.distanceToCamera = distance
			});
			// clang-format on

			Metrics::AddDrawCall(meshData->vertexCount);
		}

		const auto drawStream = m_drawStream->Build();

		ExecuteForwardPass(imageIndex, drawStream);
		ExecuteEditorPass(scene, imageIndex);
	}

	void Renderer::DrawPointLights(const Scene* scene, const uint32_t frameIndex) const
	{
		const uint32_t count = scene->GetPointLightCount();

		if (count == 0)
			return;

		const auto handle = m_pointLightBuffer.at(frameIndex);
		m_renderingDevice->CopyDataToBuffer(handle, scene->GetPointLights().data(), count * sizeof(PointLightData));
	}

	auto Renderer::CreateGlobalUniformBuffers() -> void
	{
		const uint32_t frameCount = m_renderingDevice->GetMaxFramesInFlight();

		m_globalUniformBuffer.resize(frameCount);
		for (uint32_t i = 0; i < frameCount; i++)
			m_globalUniformBuffer[i] = m_renderingDevice->CreateUniformBuffer(sizeof(GlobalUniformData));
	}

	void Renderer::CreateGlobalUniformDescriptorSets()
	{
		const auto frameCount = m_renderingDevice->GetMaxFramesInFlight();

		m_globalDescriptorSets.resize(frameCount);
		for (size_t i = 0; i < frameCount; i++)
		{
			const auto	   handle = m_globalUniformBuffer.at(i);
			constexpr auto size = sizeof(GlobalUniformData);

			m_globalDescriptorSets[i] = m_renderingDevice->CreateUniformBufferDescriptorSet(handle, size, "GlobalUniforms");
		}

		Logger::Log<SUCCESS>("Vulkan global descriptor sets: ok (count: {})", m_globalDescriptorSets.size());
	}

	auto Renderer::GetGlobalUniformBuffer(const uint32_t frameIndex) const -> GlobalUniformData*
	{
		const auto handle = m_globalUniformBuffer.at(frameIndex);
		return static_cast<GlobalUniformData*>(m_renderingDevice->GetBufferData(handle));
	}

	void Renderer::CreatePointLightBuffers()
	{
		const uint32_t	   count = m_renderingDevice->GetMaxFramesInFlight();
		constexpr uint32_t bufferSize = MAX_LIGHT_COUNT * sizeof(PointLightData);

		m_pointLightBuffer.resize(count);
		for (uint32_t i = 0; i < count; i++)
			m_pointLightBuffer[i] = m_renderingDevice->CreateStorageBuffer(bufferSize);

		Logger::Log<SUCCESS>("Point light buffers: ok (count: {})", count);
	}

	void Renderer::CreatePointLightDescriptorSets()
	{
		const auto frameCount = m_renderingDevice->GetMaxFramesInFlight();

		m_pointLightDescriptorSets.resize(frameCount);

		for (size_t i = 0; i < m_renderingDevice->GetMaxFramesInFlight(); i++)
		{
			const auto	   handle = m_pointLightBuffer.at(i);
			constexpr auto size = MAX_LIGHT_COUNT * sizeof(PointLightData);

			m_pointLightDescriptorSets[i] = m_renderingDevice->CreateStorageBufferDescriptorSet(handle, size, "PointLights");
		}

		Logger::Log<SUCCESS>("Point light descriptor sets: ok (count: {})", m_pointLightDescriptorSets.size());
	}

	void Renderer::ExecuteForwardPass(const uint32_t imageIndex, const DrawStreamView& drawStream) const
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
			barriers[0].image = m_renderingDevice->GetSwapChain()->GetImage(imageIndex);
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
			barriers[1].image = m_renderingDevice->GetSwapChain()->GetColorImage();
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
			barriers[2].image = m_renderingDevice->GetSwapChain()->GetDepthImage();
			barriers[2].subresourceRange.aspectMask = m_renderingDevice->GetSwapChain()->GetDepthAspectFlags();
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
		colorAttachment.imageView = m_renderingDevice->GetSwapChain()->GetColorImageView();
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
		colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
		colorAttachment.resolveImageView = m_renderingDevice->GetSwapChain()->GetImageView(imageIndex);
		colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkRenderingAttachmentInfo depthAttachment{};
		depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depthAttachment.imageView = m_renderingDevice->GetSwapChain()->GetDepthImageView();
		depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};
		depthAttachment.clearValue.depthStencil = {1.0f, 0};

		VkRenderingInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea = {{0, 0}, m_renderingDevice->GetSwapChain()->GetExtent()};
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;
		renderingInfo.pDepthAttachment = &depthAttachment;

		commandBuffer->ExecuteRenderPass(
			[&](RenderingApi::CommandBuffer* cmd) {
				cmd->SetViewport(0, 0, m_renderingDevice->GetSwapChain()->GetExtent(), 0, 1);
				cmd->SetScissor(0, 0, m_renderingDevice->GetSwapChain()->GetExtent());

				VkPipeline lastPipeline = VK_NULL_HANDLE;
				Handle	   lastSamplerHandle = {};

				bool boundGlobalDescriptor = false;
				bool boundPointLightDescriptor = false;

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

	void Renderer::ExecuteEditorPass(Scene* scene, uint32_t imageIndex) const
	{
		const auto commandBuffer = m_renderingDevice->GetCurrentCommandBuffer();
		const auto swapChain = m_renderingDevice->GetSwapChain();

#ifndef NDEBUG
		m_editor->RenderPanels(scene);
#endif

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

#ifndef NDEBUG
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

		commandBuffer->ExecuteRenderPass([&](const RenderingApi::CommandBuffer* cmd) { m_editor->RenderDrawData(cmd); }, renderingInfo);
#endif

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

	void Renderer::AddPipeline(const std::function<void(RenderingApi::PipelineBuilder*)>& callback)
	{
		RenderingApi::PipelineBuilder builder;
		callback(&builder);

		RenderingApi::PipelineCreateInfo params = builder.GetBuildParams();

		std::vector<const ShaderGpuData_Legacy*> shaderModules;
		shaderModules.reserve(params.shadersInfos.size());

		for (auto& [shaderPath, shaderType] : params.shadersInfos)
		{
			const auto legacyShaderId = m_resourceManager->LoadShader_Legacy(params.path + shaderPath, shaderType);
			const auto shaderGpuData = m_resourceManager->GetShader_Legacy(legacyShaderId);
			shaderModules.push_back(shaderGpuData);
		}

		params.vertexInputDescription = {};
		params.vertexInputDescription.bindings = {GetVertexBindingDescription()};
		params.vertexInputDescription.attributes = GetVertexAttributeDescriptions();

		params.device = m_renderingDevice->GetDevice();
		params.swapChain = m_renderingDevice->GetSwapChain();

		m_defaultPipeline = std::make_unique<RenderingApi::Pipeline_Legacy>(params, shaderModules);
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
