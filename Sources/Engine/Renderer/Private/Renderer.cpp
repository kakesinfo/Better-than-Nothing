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

#include "Passes/EditorPass.hpp"
#include "Passes/ForwardOpaquePass.hpp"
#include "Passes/ForwardTransparentPass.hpp"

#include <Common/Logger.hpp>
#include <Common/Metrics.hpp>
#include <Data/Graphics/DynamicUniformData.hpp>
#include <Data/Graphics/GlobalUniformData.hpp>
#include <Data/Graphics/VertexData.hpp>
#include <Math/Frustum.hpp>
#include <RenderingDevice/Blueprint/PipelineLayoutBlueprint.hpp>
#include <RenderingDevice/DrawStream.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <RenderingDevice/Types.hpp>
#include <RenderingDevice/VertexLayout.hpp>
#include <ResourceManager/PipelineBlueprint.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>
#include <Scene/Camera.hpp>
#include <Scene/Scene.hpp>
#include <Scene/SceneObject.hpp>
#include <Scene/SceneObject_Components.hpp>
#include <array>

namespace BetterThanNothing
{
	constexpr uint32_t MAX_LIGHT_COUNT = 1000;

	Renderer::Renderer(RenderingDevice* renderingDevice, ResourceManager* resourceManager, Editor* editor)
		: m_renderingDevice(renderingDevice), m_resourceManager(resourceManager), m_editor(editor)
	{
		// Set 0: Global per-frame uniforms (camera + directional light + counts)
		m_renderingDevice->CreateDescriptorSetLayout("GlobalUniforms", DescriptorType::UniformBuffer, ShaderStageFlags::Vertex | ShaderStageFlags::Fragment);

		// Set 1: Material textures + UBO (0 = albedo, 1 = normal, 2 = metallicRoughness, 3 = MaterialUniformData)
		const std::array<DescriptorBindingDesc, 4> materialBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Fragment},
			{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Fragment},
			{.binding = 2, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Fragment},
			{.binding = 3, .type = DescriptorType::UniformBuffer, .count = 1, .stages = ShaderStageFlags::Fragment},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("Material", materialBindings);

		// Set 2: Point lights SSBO
		m_renderingDevice->CreateDescriptorSetLayout("PointLights", DescriptorType::StorageBuffer, ShaderStageFlags::Fragment);

		m_renderingDevice->CreatePipelineLayout("Forward", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("Material");
			blueprint->AddDescriptorSetLayout("PointLights");
			blueprint->AddPushConstant<DynamicUniformData>(EShaderType::Vertex);
		});

		const VertexLayout vertexLayout = {VertexType::Float3, VertexType::Float3, VertexType::Float2, VertexType::Float3};

		const auto vertShader = m_resourceManager->LoadShader("Shaders/main/main.vert");
		const auto fragShader = m_resourceManager->LoadShader("Shaders/main/main.frag");

		m_resourceManager->CreatePipeline("Forward::Opaque", [&](PipelineBlueprint* blueprint) {
			blueprint->SetVertexLayout(vertexLayout)
				->AddShader(vertShader)
				->AddShader(fragShader)
				->SetPipelineLayout("Forward")
				->SetDepthTest(true)
				->SetDepthWrite(true)
				->SetBlendEnable(false)
				->SetAlphaToCoverage(true);
		});

		m_resourceManager->CreatePipeline("Forward::Transparent", [&](PipelineBlueprint* blueprint) {
			blueprint->SetVertexLayout(vertexLayout)
				->AddShader(vertShader)
				->AddShader(fragShader)
				->SetPipelineLayout("Forward")
				->SetDepthTest(true)
				->SetDepthWrite(false)
				->SetBlendEnable(true)
				->SetAlphaToCoverage(false);
		});

		m_opaqueDrawStream = std::make_unique<DrawStream>();
		m_transparentDrawStream = std::make_unique<DrawStream>();

		CreateGlobalUniformBuffers();
		CreateGlobalUniformDescriptorSets();
		CreatePointLightBuffers();
		CreatePointLightDescriptorSets();
		EnsureRenderTargets();

		AddPass(std::make_unique<ForwardOpaquePass>());
		AddPass(std::make_unique<ForwardTransparentPass>());
#ifndef NDEBUG
		if (m_editor)
			AddPass(std::make_unique<EditorPass>());
#endif

		Logger::Log<SUCCESS>("Renderer: initialized successfully ({} passes)", m_passes.size());
	}

	Renderer::~Renderer()
	{
		if (!m_renderingDevice)
			return;

		m_renderingDevice->WaitIdle();

		DestroyRenderTargets();

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

	void Renderer::AddPass(std::unique_ptr<IRenderPass> pass)
	{
		m_passes.push_back(std::move(pass));
	}

	void Renderer::EnsureRenderTargets()
	{
		const uint64_t currentGen = m_renderingDevice->GetSwapChainGeneration();
		if (currentGen == m_lastSwapChainGeneration && m_depthTarget.IsValid())
			return;

		DestroyRenderTargets();

		const auto [width, height] = m_renderingDevice->GetSwapChainExtent();
		if (width == 0 || height == 0)
			return;

		const bool useMsaa = m_renderingDevice->IsMsaaEnabled();
		if (useMsaa)
			m_colorTarget = m_renderingDevice->CreateColorAttachment(width, height, true);

		m_depthTarget = m_renderingDevice->CreateDepthAttachment(width, height, useMsaa);
		m_lastSwapChainGeneration = currentGen;
	}

	void Renderer::DestroyRenderTargets()
	{
		if (m_colorTarget.IsValid())
		{
			m_renderingDevice->DestroyTexture(m_colorTarget);
			m_colorTarget = {};
		}
		if (m_depthTarget.IsValid())
		{
			m_renderingDevice->DestroyTexture(m_depthTarget);
			m_depthTarget = {};
		}
	}

	void Renderer::RenderFrame(Scene* scene)
	{
		if (!scene || !scene->HasCamera())
			return;

		const auto imageIndexOpt = m_renderingDevice->BeginFrame();
		if (!imageIndexOpt)
			return;

		EnsureRenderTargets();

		const uint32_t imageIndex = imageIndexOpt.value();
		const uint32_t frameIndex = m_renderingDevice->GetCurrentFrameIndex();
		const auto [viewportWidth, viewportHeight] = m_renderingDevice->GetSwapChainExtent();

		BuildDrawStreams(scene, frameIndex);

		RenderPassContext ctx{};
		ctx.renderingDevice = m_renderingDevice;
		ctx.resourceManager = m_resourceManager;
		ctx.editor = m_editor;
		ctx.scene = scene;
		ctx.frameIndex = frameIndex;
		ctx.swapChainImageIndex = imageIndex;
		ctx.viewportWidth = viewportWidth;
		ctx.viewportHeight = viewportHeight;
		ctx.useMsaa = m_renderingDevice->IsMsaaEnabled();
		ctx.colorTarget = m_colorTarget;
		ctx.depthTarget = m_depthTarget;
		ctx.opaqueStream = m_opaqueDrawStream.get();
		ctx.transparentStream = m_transparentDrawStream.get();

		for (const auto& pass : m_passes)
			pass->Execute(ctx);

		m_renderingDevice->EndFrame(imageIndex);
	}

	void Renderer::BuildDrawStreams(Scene* scene, const uint32_t frameIndex)
	{
		const float	  aspectRatio = m_renderingDevice->GetAspectRatio();
		const Camera* camera = scene->GetCamera();

		Vector3 dirLightDirection = Vector3(0.0f, -1.0f, 0.0f);
		Vector3 dirLightColor = Vector3::One;
		float	dirLightIntensity = 0.0f;
		if (const auto dirLightView = scene->GetView<DirectionalLightComponent>(); dirLightView.begin() != dirLightView.end())
		{
			const auto& dirLight = dirLightView.get<DirectionalLightComponent>(*dirLightView.begin());
			dirLightDirection = dirLight.direction;
			dirLightColor = dirLight.color;
			dirLightIntensity = dirLight.illuminanceLux;
		}

		const uint32_t pointLightCount = UploadPointLights(scene, frameIndex);

		const Matrix4& viewMatrix = camera->GetViewMatrix();
		const Matrix4  projMatrix = camera->GetProjectionMatrix(aspectRatio);
		const Matrix4  unjitteredViewProj = projMatrix * viewMatrix;
		const Matrix4  viewProj = unjitteredViewProj;
		const Frustum  frustum = Frustum::FromViewProjection(viewProj);

		const auto [swapW, swapH] = m_renderingDevice->GetSwapChainExtent();

		const auto globalUniform = GetGlobalUniformBuffer(frameIndex);
		globalUniform->view = viewMatrix;
		globalUniform->projection = projMatrix;
		globalUniform->viewProj = viewProj;
		globalUniform->invViewProj = viewProj.Inverted();
		globalUniform->unjitteredViewProj = unjitteredViewProj;
		globalUniform->prevUnjitteredViewProj = m_frameIndex == 0 ? unjitteredViewProj : m_prevUnjitteredViewProj;
		globalUniform->cameraPosition = camera->GetPosition();
		globalUniform->cameraExposure = camera->GetExposure();
		globalUniform->lightDirection = dirLightDirection;
		globalUniform->lightColor = dirLightColor;
		globalUniform->lightIntensity = dirLightIntensity;
		globalUniform->pointLightCount = pointLightCount;
		globalUniform->screenWidth = static_cast<float>(swapW);
		globalUniform->screenHeight = static_cast<float>(swapH);
		globalUniform->frameIndex = m_frameIndex;

		m_prevUnjitteredViewProj = unjitteredViewProj;
		++m_frameIndex;

		Metrics::SetCameraCoordinates(camera->GetPosition().GetX(), camera->GetPosition().GetY(), camera->GetPosition().GetZ());

		const auto meshView = scene->GetView<TransformComponent, MeshComponent>();
		const auto estimatedCount = static_cast<uint32_t>(meshView.size_hint());
		m_opaqueDrawStream->Reset(estimatedCount);
		m_transparentDrawStream->Reset(estimatedCount / 4);

		const Handle  globalSetHandle = m_globalDescriptorSets[frameIndex];
		const Handle  pointLightSetHandle = m_pointLightDescriptorSets[frameIndex];
		const Vector3 cameraPos = camera->GetPosition();

		for (auto [entity, transform, meshComp] : meshView.each())
		{
			if (meshComp.meshId == InvalidResourceId)
				continue;

			const MeshResourceCache* meshData = m_resourceManager->GetMeshData(meshComp.meshId);
			if (!meshData)
				continue;

			// Frustum culling against mesh world-space AABB
			if (!frustum.IntersectsLocalAABB(meshData->boundsMin, meshData->boundsMax, transform.worldMatrix))
			{
				Metrics::AddCulledDrawCall();
				continue;
			}

			// Resolve Material (lazy fallback to default material if entity only set textureId)
			ResourceId materialId = meshComp.materialId;
			if (materialId == InvalidResourceId)
			{
				MaterialCreateInfo fallbackMat{};
				fallbackMat.diffuseTextureId = meshComp.textureId;
				materialId = m_resourceManager->CreateMaterial(fallbackMat);
				meshComp.materialId = materialId;
			}

			const MaterialResourceCache* material = m_resourceManager->GetMaterialData(materialId);
			const PipelineResourceCache* pipeline = m_resourceManager->GetPipelineData(material->pipelineName);

			const float distance = Vector3::Distance2(cameraPos, transform.GetWorldPosition());
			const bool	isTransparent = (material->alphaMode == EAlphaMode::Blend);

			DrawPacket packet{};
			packet.pipeline = pipeline->pipeline;
			packet.pipelineLayoutName = pipeline->layoutName;
			packet.vertexBuffer = meshData->vertexBuffer;
			packet.indexBuffer = meshData->indexBuffer;
			packet.indexCount = meshData->indexCount;
			packet.descriptorSets[0] = globalSetHandle;
			packet.descriptorSets[1] = material->descriptorSetHandle;
			packet.descriptorSets[2] = pointLightSetHandle;
			packet.descriptorSetCount = 3;
			packet.SetPushConstant(DynamicUniformData{transform.worldMatrix, transform.prevWorldMatrix}, ShaderStageFlags::Vertex);
			packet.distanceToCamera = distance;
			packet.sortBackToFront = isTransparent;

			if (isTransparent)
				m_transparentDrawStream->Draw(packet);
			else
				m_opaqueDrawStream->Draw(packet);

			Metrics::AddDrawCall(meshData->vertexCount);
		}
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
			data.radius = light.radius;
			data.color = light.color;
			data.luminousPowerLm = light.luminousPowerLm;
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

	void Renderer::CreateGlobalUniformBuffers()
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
		for (size_t i = 0; i < frameCount; i++)
		{
			const auto	   handle = m_pointLightBuffer.at(i);
			constexpr auto size = MAX_LIGHT_COUNT * sizeof(PointLightData);

			m_pointLightDescriptorSets[i] = m_renderingDevice->CreateStorageBufferDescriptorSet(handle, size, "PointLights");
		}

		Logger::Log<SUCCESS>("Point light descriptor sets: ok (count: {})", m_pointLightDescriptorSets.size());
	}
}
