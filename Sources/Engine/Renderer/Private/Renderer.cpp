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

#include <Common/Logger.hpp>
#include <Common/Metrics.hpp>
#include <Data/Graphics/CameraData.hpp>
#include <Data/Graphics/DirectionalLightData.hpp>
#include <Data/Graphics/DynamicUniformData.hpp>
#include <Data/Graphics/GlobalUniformData.hpp>
#include <Data/Graphics/VertexData.hpp>
#include <Editor/Editor.hpp>
#include <RenderingApi.hpp>
#include <RenderingApi/Core/PipelineLayout.hpp>
#include <RenderingApi/Resources/Buffer.hpp>
#include <RenderingApi/Resources/DescriptorSet.hpp>
#include <RenderingApi/Resources/Pipeline.hpp>
#include <RenderingDevice/Blueprint/PipelineLayoutBlueprint.hpp>
#include <RenderingDevice/DrawStream.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <RenderingDevice/VertexLayout.hpp>
#include <ResourceManager/PipelineBlueprint.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>
#include <Scene/Camera.hpp>
#include <Scene/Scene.hpp>
#include <Scene/SceneObject.hpp>
#include <Scene/SceneObject_Components.hpp>
#include <optional>

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

		const VertexLayout vertexLayout = {VertexType::Float3, VertexType::Float3, VertexType::Float2, VertexType::Float3};

		const auto vertShader = m_resourceManager->LoadShader("../../../Shaders/main/main.vert");
		const auto fragShader = m_resourceManager->LoadShader("../../../Shaders/main/main.frag");

		m_resourceManager->CreatePipeline("Forward::Opaque", [&](PipelineBlueprint* blueprint) {
			blueprint->SetVertexLayout(vertexLayout);
			blueprint->AddShader(vertShader);
			blueprint->AddShader(fragShader);
			blueprint->SetPipelineLayout("Forward");
		});

		m_drawStream = std::make_unique<DrawStream>();

		CreateGlobalUniformBuffers();
		CreateGlobalUniformDescriptorSets();
		CreatePointLightBuffers();
		CreatePointLightDescriptorSets();

		Logger::Log<SUCCESS>("Renderer: initialized successfully");
	}

	Renderer::~Renderer()
	{
		if (!m_renderingDevice)
			return;

		m_renderingDevice->WaitIdle();

		for (const Handle& handle : m_globalDescriptorSets)
		{
			if (handle.IsValid())
				m_renderingDevice->DestroyDescriptorSet(handle);
		}
		for (const Handle& handle : m_globalUniformBuffer)
		{
			if (handle.IsValid())
				m_renderingDevice->DestroyBuffer(handle);
		}
		for (const Handle& handle : m_pointLightDescriptorSets)
		{
			if (handle.IsValid())
				m_renderingDevice->DestroyDescriptorSet(handle);
		}
		for (const Handle& handle : m_pointLightBuffer)
		{
			if (handle.IsValid())
				m_renderingDevice->DestroyBuffer(handle);
		}
	}

	std::unique_ptr<Renderer> Renderer::Create(RenderingDevice* renderingDevice, ResourceManager* resourceManager, Editor* editor)
	{
		return std::make_unique<Renderer>(renderingDevice, resourceManager, editor);
	}

	void Renderer::RenderFrame(Scene* scene) const
	{
		if (!scene || !scene->HasCamera())
			return;

		const auto imageIndexOpt = m_renderingDevice->BeginFrame();
		if (!imageIndexOpt)
			return;

		const uint32_t imageIndex = imageIndexOpt.value();

		RecordCommandBuffer(scene, imageIndex);

		m_renderingDevice->EndFrame(imageIndex);
	}

	void Renderer::RecordCommandBuffer(Scene* scene, const uint32_t imageIndex) const
	{
		const uint32_t frameIndex = m_renderingDevice->GetCurrentFrameIndex();
		const float	   aspectRatio = m_renderingDevice->GetAspectRatio();

		const Camera* camera = scene->GetCamera();

		// Fetch first directional light in ECS, or fallback to zero intensity
		Vector3 dirLightDirection = Vector3(0.0f, -1.0f, 0.0f);
		Vector3 dirLightColor = Vector3::One;
		float	dirLightIntensity = 0.0f;
		if (const auto dirLightView = scene->GetView<DirectionalLightComponent>(); dirLightView.begin() != dirLightView.end())
		{
			const auto& dirLight = dirLightView.get<DirectionalLightComponent>(*dirLightView.begin());
			dirLightDirection = dirLight.direction;
			dirLightColor = dirLight.color;
			dirLightIntensity = dirLight.intensity;
		}

		const uint32_t pointLightCount = UploadPointLights(scene, frameIndex);

		const auto globalUniform = GetGlobalUniformBuffer(frameIndex);
		globalUniform->view = camera->GetViewMatrix();
		globalUniform->projection = camera->GetProjectionMatrix(aspectRatio);
		globalUniform->cameraPosition = camera->GetPosition();
		globalUniform->cameraExposure = camera->GetExposure();
		globalUniform->lightDirection = dirLightDirection;
		globalUniform->lightColor = dirLightColor;
		globalUniform->lightIntensity = dirLightIntensity;
		globalUniform->pointLightCount = pointLightCount;

		Metrics::SetCameraCoordinates(camera->GetPosition().GetX(), camera->GetPosition().GetY(), camera->GetPosition().GetZ());

		const auto meshView = scene->GetView<TransformComponent, MeshComponent>();
		m_drawStream->Reset(static_cast<uint32_t>(meshView.size_hint()));

		const auto			   defaultPipeline = m_resourceManager->GetPipelineData("Forward::Opaque");
		const VkPipeline	   vkPipeline = m_renderingDevice->GetPipeline(defaultPipeline->pipeline)->pipeline;
		const VkPipelineLayout vkPipelineLayout = m_renderingDevice->GetPipelineLayout(defaultPipeline->layoutName)->Handle();
		const VkDescriptorSet  globalDescSet = m_renderingDevice->GetDescriptorSet(m_globalDescriptorSets[frameIndex])->value;
		const VkDescriptorSet  pointLightDescSet = m_renderingDevice->GetDescriptorSet(m_pointLightDescriptorSets[frameIndex])->value;
		const Vector3		   cameraPos = camera->GetPosition();

		for (auto [entity, transform, meshComp] : meshView.each())
		{
			if (meshComp.meshId == InvalidResourceId || meshComp.textureId == InvalidResourceId)
				continue;

			const auto meshData = m_resourceManager->GetMeshData(meshComp.meshId);
			const auto textureData = m_resourceManager->GetTextureData(meshComp.textureId);

			if (!meshData || !textureData)
				continue;

			const float distance = Vector3::Distance2(cameraPos, transform.GetWorldPosition());

			// clang-format off
			m_drawStream->Draw({
				.pipelineHandle = defaultPipeline->pipeline,
				.pipeline = vkPipeline,
				.pipelineLayout = vkPipelineLayout,
				.vertexBuffer = meshData->vertexBuffer,
				.indexBuffer = meshData->indexBuffer,
				.indexCount = meshData->indexCount,
				.globalDescriptorSet = globalDescSet,
				.samplerDescriptorSet = textureData->descriptorSetHandle,
				.pointLightDescriptorSet = pointLightDescSet,
				.transform = transform.worldMatrix,
				.distanceToCamera = distance
			});
			// clang-format on

			Metrics::AddDrawCall(meshData->vertexCount);
		}

		const auto drawStream = m_drawStream->Build();

		ExecuteForwardPass(imageIndex, drawStream);
		ExecuteEditorPass(scene, imageIndex);
	}

	uint32_t Renderer::UploadPointLights(const Scene* scene, const uint32_t frameIndex) const
	{
		m_pointLightsScratch.clear();

		const auto lightView = scene->GetView<TransformComponent, PointLightComponent>();
		for (auto [entity, transform, light] : lightView.each())
		{
			if (m_pointLightsScratch.size() >= MAX_LIGHT_COUNT)
			{
				if (!m_warnedLightLimit)
				{
					Logger::Log<WARNING>("Point light count exceeds MAX_LIGHT_COUNT ({}); extra lights will be ignored", MAX_LIGHT_COUNT);
					m_warnedLightLimit = true;
				}
				break;
			}

			PointLightData data{};
			data.position = transform.GetWorldPosition();
			data.color = light.color;
			data.constant = light.constant;
			data.linear = light.linear;
			data.quadratic = light.quadratic;
			data.intensity = light.intensity;
			m_pointLightsScratch.push_back(data);
		}

		const auto count = static_cast<uint32_t>(m_pointLightsScratch.size());
		if (count > 0)
		{
			const auto handle = m_pointLightBuffer.at(frameIndex);
			m_renderingDevice->CopyDataToBuffer(handle, m_pointLightsScratch.data(), count * sizeof(PointLightData));
		}

		return count;
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
		const auto swapChain = m_renderingDevice->GetSwapChain();
		const bool useMsaa = m_renderingDevice->GetDevice()->GetMaxMsaaSamples() > VK_SAMPLE_COUNT_1_BIT;

		{
			std::array<VkImageMemoryBarrier, 3> barriers{};
			uint32_t							barrierCount = 0;

			auto& swapchainBarrier = barriers[barrierCount++];
			swapchainBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			swapchainBarrier.srcAccessMask = 0;
			swapchainBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			swapchainBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			swapchainBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			swapchainBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			swapchainBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			swapchainBarrier.image = swapChain->GetImage(imageIndex);
			swapchainBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

			if (useMsaa)
			{
				auto& msaaBarrier = barriers[barrierCount++];
				msaaBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				msaaBarrier.srcAccessMask = 0;
				msaaBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
				msaaBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				msaaBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				msaaBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				msaaBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				msaaBarrier.image = swapChain->GetColorImage();
				msaaBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
			}

			auto& depthBarrier = barriers[barrierCount++];
			depthBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			depthBarrier.srcAccessMask = 0;
			depthBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			depthBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			depthBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			depthBarrier.image = swapChain->GetDepthImage();
			depthBarrier.subresourceRange = {swapChain->GetDepthAspectFlags(), 0, 1, 0, 1};

			commandBuffer->PipelineBarrier(
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr,
				barrierCount, barriers.data()
			);
		}

		VkRenderingAttachmentInfo colorAttachment{};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
		if (useMsaa)
		{
			colorAttachment.imageView = swapChain->GetColorImageView();
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
			colorAttachment.resolveImageView = swapChain->GetImageView(imageIndex);
			colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		else
		{
			colorAttachment.imageView = swapChain->GetImageView(imageIndex);
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
		}

		VkRenderingAttachmentInfo depthAttachment{};
		depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depthAttachment.imageView = swapChain->GetDepthImageView();
		depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.clearValue.depthStencil = {1.0f, 0};

		VkRenderingInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea = {{0, 0}, swapChain->GetExtent()};
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;
		renderingInfo.pDepthAttachment = &depthAttachment;

		commandBuffer->ExecuteRenderPass(
			[&](RenderingApi::CommandBuffer* cmd) {
				cmd->SetViewport(0, 0, swapChain->GetExtent(), 0, 1);
				cmd->SetScissor(0, 0, swapChain->GetExtent());

				VkPipeline		 lastPipeline = VK_NULL_HANDLE;
				VkPipelineLayout lastPipelineLayout = VK_NULL_HANDLE;
				Handle			 lastSamplerHandle{};
				Handle			 lastVertexBuffer{};
				Handle			 lastIndexBuffer{};

				for (const DrawPacket& drawPacket : drawStream)
				{
					if (lastPipeline != drawPacket.pipeline)
					{
						cmd->BindPipeline(drawPacket.pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
						lastPipeline = drawPacket.pipeline;
					}

					if (lastPipelineLayout != drawPacket.pipelineLayout)
					{
						cmd->BindDescriptorSets(drawPacket.pipelineLayout, 0, 1, &drawPacket.globalDescriptorSet, 0, nullptr);
						cmd->BindDescriptorSets(drawPacket.pipelineLayout, 2, 1, &drawPacket.pointLightDescriptorSet, 0, nullptr);
						lastPipelineLayout = drawPacket.pipelineLayout;
						lastSamplerHandle = {};
					}

					if (lastSamplerHandle != drawPacket.samplerDescriptorSet)
					{
						const auto samplerDescriptorSet = m_renderingDevice->GetDescriptorSet(drawPacket.samplerDescriptorSet);
						cmd->BindDescriptorSets(drawPacket.pipelineLayout, 1, 1, &samplerDescriptorSet->value, 0, nullptr);
						lastSamplerHandle = drawPacket.samplerDescriptorSet;
					}

					const DynamicUniformData dynamicUniforms = {drawPacket.transform};
					cmd->PushConstants(drawPacket.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(DynamicUniformData), &dynamicUniforms);

					if (lastVertexBuffer != drawPacket.vertexBuffer)
					{
						const auto			   vertexBuffer = m_renderingDevice->GetBuffer(drawPacket.vertexBuffer);
						const VkBuffer		   vertexBuffers[] = {vertexBuffer->value};
						constexpr VkDeviceSize offsets[] = {0};
						cmd->BindVertexBuffers(0, 1, vertexBuffers, offsets);
						lastVertexBuffer = drawPacket.vertexBuffer;
					}

					if (lastIndexBuffer != drawPacket.indexBuffer)
					{
						const auto indexBuffer = m_renderingDevice->GetBuffer(drawPacket.indexBuffer);
						cmd->BindIndexBuffer(indexBuffer->value, 0, VK_INDEX_TYPE_UINT32);
						lastIndexBuffer = drawPacket.indexBuffer;
					}

					cmd->DrawIndexed(drawPacket.indexCount, 1, 0, 0, 0);
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
		if (m_editor)
		{
			m_editor->RenderPanels(scene);

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
				barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

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

			commandBuffer->ExecuteRenderPass([&](const RenderingApi::CommandBuffer* cmd) { m_editor->RenderDrawData(cmd); }, renderingInfo);
		}
#else
		(void) scene;
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
			presentationBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

			commandBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentationBarrier);
		}
	}
}
