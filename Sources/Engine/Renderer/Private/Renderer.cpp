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
#include "Renderer/RenderSettings.hpp"

#include "Passes/DeferredLightingPass.hpp"
#include "Passes/AOPass.hpp"
#include "Passes/EditorPass.hpp"
#include "Passes/ForwardOpaquePass.hpp"
#include "Passes/ForwardTransparentPass.hpp"
#include "Passes/GBufferPass.hpp"
#include "Passes/PostProcessPass.hpp"
#include "Passes/RTShadowPass.hpp"
#include "Passes/ScreenSpaceShadowPass.hpp"

#include <Common/Logger.hpp>
#include <Common/Metrics.hpp>
#include <Data/Graphics/DynamicUniformData.hpp>
#include <Data/Graphics/GlobalUniformData.hpp>
#include <Data/Graphics/VertexData.hpp>
#include <Math/Frustum.hpp>
#include <RenderingDevice/Blueprint/PipelineLayoutBlueprint.hpp>
#include <RenderingDevice/BufferDescription.hpp>
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
		m_renderingDevice->CreateDescriptorSetLayout("GlobalUniforms", DescriptorType::UniformBuffer, ShaderStageFlags::Vertex | ShaderStageFlags::Fragment | ShaderStageFlags::Compute);

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
				->SetColorFormat(TextureFormat::R16G16B16A16_SFLOAT)
				->SetDepthFormat(TextureFormat::D32_SFLOAT)
				->SetUseMsaa(false)
				->SetDepthTest(true)
				->SetDepthWrite(false)
				->SetBlendEnable(true)
				->SetAlphaToCoverage(false);
		});

		const auto gbufferVert = m_resourceManager->LoadShader("Shaders/gbuffer/gbuffer.vert");
		const auto gbufferFrag = m_resourceManager->LoadShader("Shaders/gbuffer/gbuffer.frag");
		const std::vector<TextureFormat> gbufferFormats = {
			TextureFormat::R8G8B8A8_UNORM,
			TextureFormat::R16G16_SFLOAT,
			TextureFormat::R8G8B8A8_UNORM,
			TextureFormat::R16G16_SFLOAT,
		};

		m_resourceManager->CreatePipeline("GBuffer::Opaque", [&](PipelineBlueprint* blueprint) {
			blueprint->SetVertexLayout(vertexLayout)
				->AddShader(gbufferVert)
				->AddShader(gbufferFrag)
				->SetPipelineLayout("Forward")
				->SetColorFormats(gbufferFormats)
				->SetDepthFormat(TextureFormat::D32_SFLOAT)
				->SetUseMsaa(false)
				->SetBlendEnable(false)
				->SetAlphaToCoverage(false)
				->SetDepthTest(true)
				->SetDepthWrite(true)
				->SetCullMode(CullMode::Back);
		});

		m_resourceManager->CreatePipeline("GBuffer::Masked", [&](PipelineBlueprint* blueprint) {
			blueprint->SetVertexLayout(vertexLayout)
				->AddShader(gbufferVert)
				->AddShader(gbufferFrag)
				->SetPipelineLayout("Forward")
				->SetColorFormats(gbufferFormats)
				->SetDepthFormat(TextureFormat::D32_SFLOAT)
				->SetUseMsaa(false)
				->SetBlendEnable(false)
				->SetAlphaToCoverage(false)
				->SetDepthTest(true)
				->SetDepthWrite(true)
				->SetCullMode(CullMode::None);
		});

		const std::array<DescriptorBindingDesc, 10> deferredBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 2, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 3, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 4, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 5, .type = DescriptorType::StorageBuffer, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 6, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 7, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 8, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 9, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("DeferredLighting", deferredBindings);

		m_renderingDevice->CreatePipelineLayout("DeferredLighting", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("DeferredLighting");
		});

		const auto deferredComp = m_resourceManager->LoadShader("Shaders/deferred/deferred_lighting.comp");
		m_resourceManager->CreateComputePipeline("Deferred::Lighting", deferredComp, "DeferredLighting");

		SamplerDescription computeSamplerDesc{};
		computeSamplerDesc.magFilter = SamplerFilter::Nearest;
		computeSamplerDesc.minFilter = SamplerFilter::Nearest;
		computeSamplerDesc.addressModeU = SamplerAddressMode::ClampToEdge;
		computeSamplerDesc.addressModeV = SamplerAddressMode::ClampToEdge;
		computeSamplerDesc.addressModeW = SamplerAddressMode::ClampToEdge;
		m_computeNearestSampler = m_renderingDevice->GetOrCreateTextureSampler(computeSamplerDesc);

		SamplerDescription linearClampDesc{};
		linearClampDesc.magFilter = SamplerFilter::Linear;
		linearClampDesc.minFilter = SamplerFilter::Linear;
		linearClampDesc.addressModeU = SamplerAddressMode::ClampToEdge;
		linearClampDesc.addressModeV = SamplerAddressMode::ClampToEdge;
		linearClampDesc.addressModeW = SamplerAddressMode::ClampToEdge;
		m_linearClampSampler = m_renderingDevice->GetOrCreateTextureSampler(linearClampDesc);

		m_pointBorderWhiteSampler = m_renderingDevice->GetOrCreateTextureSampler(MakePointBorderWhiteSamplerDesc());

		const std::array<DescriptorBindingDesc, 4> taaBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 2, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 3, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("PostProcess::TAA", taaBindings);
		m_renderingDevice->CreatePipelineLayout("PostProcess::TAA", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("PostProcess::TAA");
		});
		const auto taaComp = m_resourceManager->LoadShader("Shaders/postprocess/taa.comp");
		m_resourceManager->CreateComputePipeline("PostProcess::TAA", taaComp, "PostProcess::TAA");

		// RT Shadow Trace
		const std::array<DescriptorBindingDesc, 5> rtShadowTraceBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 2, .type = DescriptorType::AccelerationStructure, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 3, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 4, .type = DescriptorType::StorageBuffer, .count = 1, .stages = ShaderStageFlags::Compute},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("RTShadow::Trace", rtShadowTraceBindings);
		m_renderingDevice->CreatePipelineLayout("RTShadow::Trace", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("RTShadow::Trace");
			blueprint->AddPushConstant<RTShadowPushConstants>(EShaderType::Compute);
		});
		const auto rtShadowTraceComp = m_resourceManager->LoadShader("Shaders/rtshadows/rt_shadows.comp");
		m_resourceManager->CreateComputePipeline("RTShadow::Trace", rtShadowTraceComp, "RTShadow::Trace");

		// FFX Shadows Tile Classification
		const std::array<DescriptorBindingDesc, 11> rtShadowClassifyBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 2, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 3, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 4, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 5, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 6, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 7, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 8, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 9, .type = DescriptorType::StorageBuffer, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 10, .type = DescriptorType::StorageBuffer, .count = 1, .stages = ShaderStageFlags::Compute},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("RTShadow::FFXTileClassification", rtShadowClassifyBindings);
		m_renderingDevice->CreatePipelineLayout("RTShadow::FFXTileClassification", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("RTShadow::FFXTileClassification");
		});
		const auto ffxClassifyComp = m_resourceManager->LoadShader("Shaders/rtshadows/ffx_shadows_tile_classification.comp");
		m_resourceManager->CreateComputePipeline("RTShadow::FFXTileClassification", ffxClassifyComp, "RTShadow::FFXTileClassification");

		// FFX Shadows Spatial Filter
		const std::array<DescriptorBindingDesc, 5> rtShadowFilterBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 2, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 3, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 4, .type = DescriptorType::StorageBuffer, .count = 1, .stages = ShaderStageFlags::Compute},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("RTShadow::FFXFilterSoftShadows", rtShadowFilterBindings);
		m_renderingDevice->CreatePipelineLayout("RTShadow::FFXFilterSoftShadows", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("RTShadow::FFXFilterSoftShadows");
			blueprint->AddPushConstant<int32_t>(EShaderType::Compute);
		});
		const auto ffxFilterComp = m_resourceManager->LoadShader("Shaders/rtshadows/ffx_shadows_filter.comp");
		m_resourceManager->CreateComputePipeline("RTShadow::FFXFilterSoftShadows", ffxFilterComp, "RTShadow::FFXFilterSoftShadows");

		// Bend Studio Screen-Space Shadows
		const std::array<DescriptorBindingDesc, 2> sssBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 1, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("SSS::Bend", sssBindings);
		m_renderingDevice->CreatePipelineLayout("SSS::Bend", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("SSS::Bend");
			blueprint->AddPushConstant<BendSSSPushConstants>(EShaderType::Compute);
		});
		const auto bendSssComp = m_resourceManager->LoadShader("Shaders/rtshadows/bend_sss.comp");
		m_resourceManager->CreateComputePipeline("SSS::Bend", bendSssComp, "SSS::Bend");

		// Ambient Occlusion (RTAO, GTAO, Denoiser)
		const std::array<DescriptorBindingDesc, 4> rtaoBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 2, .type = DescriptorType::AccelerationStructure, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 3, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("AO::RTAO", rtaoBindings);
		m_renderingDevice->CreatePipelineLayout("AO::RTAO", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("AO::RTAO");
			blueprint->AddPushConstant<AOPushConstants>(EShaderType::Compute);
		});
		const auto rtaoComp = m_resourceManager->LoadShader("Shaders/ao/rtao.comp");
		m_resourceManager->CreateComputePipeline("AO::RTAO", rtaoComp, "AO::RTAO");

		const std::array<DescriptorBindingDesc, 3> gtaoBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 2, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("AO::GTAO", gtaoBindings);
		m_renderingDevice->CreatePipelineLayout("AO::GTAO", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("AO::GTAO");
			blueprint->AddPushConstant<AOPushConstants>(EShaderType::Compute);
		});
		const auto gtaoComp = m_resourceManager->LoadShader("Shaders/ao/gtao.comp");
		m_resourceManager->CreateComputePipeline("AO::GTAO", gtaoComp, "AO::GTAO");

		const std::array<DescriptorBindingDesc, 8> aoDenoiseBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 2, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 3, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 4, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 5, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 6, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 7, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("AO::Denoise", aoDenoiseBindings);
		m_renderingDevice->CreatePipelineLayout("AO::Denoise", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("AO::Denoise");
		});
		const auto aoDenoiseComp = m_resourceManager->LoadShader("Shaders/ao/ao_denoise.comp");
		m_resourceManager->CreateComputePipeline("AO::Denoise", aoDenoiseComp, "AO::Denoise");

		m_renderingDevice->CreateDescriptorSetLayout("PostProcess::ToneMap", DescriptorType::CombinedImageSampler, ShaderStageFlags::Fragment);
		m_renderingDevice->CreatePipelineLayout("PostProcess::ToneMap", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("PostProcess::ToneMap");
		});
		const auto tonemapVert = m_resourceManager->LoadShader("Shaders/postprocess/tonemap.vert");
		const auto tonemapFrag = m_resourceManager->LoadShader("Shaders/postprocess/tonemap.frag");
		m_resourceManager->CreatePipeline("PostProcess::ToneMap", [&](PipelineBlueprint* blueprint) {
			blueprint->SetVertexLayout(VertexLayout{})
				->AddShader(tonemapVert)
				->AddShader(tonemapFrag)
				->SetPipelineLayout("PostProcess::ToneMap")
				->SetColorFormat(TextureFormat::Undefined)
				->SetDepthFormat(TextureFormat::Undefined)
				->SetUseMsaa(false)
				->SetDepthTest(false)
				->SetDepthWrite(false)
				->SetBlendEnable(false)
				->SetAlphaToCoverage(false)
				->SetCullMode(CullMode::None);
		});

		m_opaqueDrawStream = std::make_unique<DrawStream>();
		m_transparentDrawStream = std::make_unique<DrawStream>();

		CreateGlobalUniformBuffers();
		CreateGlobalUniformDescriptorSets();
		CreatePointLightBuffers();
		CreatePointLightDescriptorSets();
		CreateAtmosphereLuts();
		EnsureRenderTargets();

		AddPass(std::make_unique<GBufferPass>());
		AddPass(std::make_unique<RTShadowPass>());
		AddPass(std::make_unique<ScreenSpaceShadowPass>());
		AddPass(std::make_unique<AOPass>());
		AddPass(std::make_unique<DeferredLightingPass>());
		AddPass(std::make_unique<ForwardTransparentPass>());
		AddPass(std::make_unique<PostProcessPass>());
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

		DestroyAtmosphereLuts();
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
		for (const Handle& handle : m_deferredDescriptorSets)
		{
			if (handle.IsValid())
				m_renderingDevice->DestroyDescriptorSet(handle);
		}
		if (m_computeNearestSampler.IsValid())
			m_renderingDevice->DestroyTextureSampler(m_computeNearestSampler);
		if (m_linearClampSampler.IsValid())
			m_renderingDevice->DestroyTextureSampler(m_linearClampSampler);
		if (m_pointBorderWhiteSampler.IsValid())
			m_renderingDevice->DestroyTextureSampler(m_pointBorderWhiteSampler);
		if (m_skyViewSampler.IsValid())
			m_renderingDevice->DestroyTextureSampler(m_skyViewSampler);
		for (uint32_t p = 0; p < 2; ++p)
		{
			for (const Handle& handle : m_taaDescriptorSets[p])
			{
				if (handle.IsValid())
					m_renderingDevice->DestroyDescriptorSet(handle);
			}
			for (const Handle& handle : m_tonemapDescriptorSets[p])
			{
				if (handle.IsValid())
					m_renderingDevice->DestroyDescriptorSet(handle);
			}
			for (const Handle& handle : m_rtShadowClassifyDescriptorSets[p])
			{
				if (handle.IsValid())
					m_renderingDevice->DestroyDescriptorSet(handle);
			}
		}
		for (const Handle& handle : m_rtShadowTraceDescriptorSets)
		{
			if (handle.IsValid())
				m_renderingDevice->DestroyDescriptorSet(handle);
		}
		for (uint32_t f = 0; f < 3; ++f)
		{
			for (const Handle& handle : m_rtShadowFilterDescriptorSets[f])
			{
				if (handle.IsValid())
					m_renderingDevice->DestroyDescriptorSet(handle);
			}
		}
		for (const Handle& handle : m_sssDescriptorSets)
		{
			if (handle.IsValid())
				m_renderingDevice->DestroyDescriptorSet(handle);
		}
		for (const Handle& handle : m_rtaoDescriptorSets)
		{
			if (handle.IsValid())
				m_renderingDevice->DestroyDescriptorSet(handle);
		}
		for (const Handle& handle : m_gtaoDescriptorSets)
		{
			if (handle.IsValid())
				m_renderingDevice->DestroyDescriptorSet(handle);
		}
		for (const Handle& handle : m_aoDenoiseDescriptorSets)
		{
			if (handle.IsValid())
				m_renderingDevice->DestroyDescriptorSet(handle);
		}
	}

	std::unique_ptr<Renderer> Renderer::Create(RenderingDevice* renderingDevice, ResourceManager* resourceManager, Editor* editor)
	{
		return std::make_unique<Renderer>(renderingDevice, resourceManager, editor);
	}

	float Renderer::Halton(uint32_t index, const uint32_t base)
	{
		float f = 1.0f;
		float r = 0.0f;
		while (index > 0)
		{
			f /= static_cast<float>(base);
			r += f * static_cast<float>(index % base);
			index /= base;
		}
		return r;
	}

	uint32_t Renderer::ComputeRTShadowTileCount(const uint32_t width, const uint32_t height)
	{
		const uint32_t tilesX = (width + 7u) / 8u;
		const uint32_t tilesY = (height + 3u) / 4u;
		return tilesX * tilesY;
	}

	SamplerDescription Renderer::MakePointBorderWhiteSamplerDesc()
	{
		SamplerDescription desc{};
		desc.magFilter = SamplerFilter::Nearest;
		desc.minFilter = SamplerFilter::Nearest;
		desc.mipmapMode = SamplerMipmapMode::Nearest;
		desc.addressModeU = SamplerAddressMode::ClampToBorder;
		desc.addressModeV = SamplerAddressMode::ClampToBorder;
		desc.addressModeW = SamplerAddressMode::ClampToBorder;
		desc.anisotropyEnable = false;
		desc.borderColor = BorderColor::FloatOpaqueWhite;
		return desc;
	}

	SamplerDescription Renderer::MakeSkyViewSamplerDesc()
	{
		SamplerDescription desc{};
		desc.magFilter = SamplerFilter::Linear;
		desc.minFilter = SamplerFilter::Linear;
		desc.mipmapMode = SamplerMipmapMode::Linear;
		desc.addressModeU = SamplerAddressMode::Repeat;
		desc.addressModeV = SamplerAddressMode::ClampToEdge;
		desc.addressModeW = SamplerAddressMode::ClampToEdge;
		desc.anisotropyEnable = false;
		return desc;
	}

	AtmosphereLutSpec Renderer::GetTransmittanceLutSpec() { return {256, 64, TextureFormat::R16G16B16A16_SFLOAT}; }
	AtmosphereLutSpec Renderer::GetMultiScatteringLutSpec() { return {32, 32, TextureFormat::R16G16B16A16_SFLOAT}; }
	AtmosphereLutSpec Renderer::GetSkyViewLutSpec() { return {192, 108, TextureFormat::R16G16B16A16_SFLOAT}; }
	AtmosphereLutSpec Renderer::GetIrradianceLutSpec() { return {64, 32, TextureFormat::R16G16B16A16_SFLOAT}; }
	AtmosphereLutSpec Renderer::GetPrefilteredEnvLutSpec() { return {128, 320, TextureFormat::R16G16B16A16_SFLOAT}; }
	AtmosphereLutSpec Renderer::GetBrdfLutSpec() { return {256, 256, TextureFormat::R16G16_SFLOAT}; }

	void Renderer::CreateAtmosphereLuts()
	{
		m_skyViewSampler = m_renderingDevice->GetOrCreateTextureSampler(MakeSkyViewSamplerDesc());

		auto createLut = [&](const AtmosphereLutSpec& spec) -> Handle {
			TextureCreateInfo desc{};
			desc.width = spec.width;
			desc.height = spec.height;
			desc.format = spec.format;
			desc.usage = TextureUsageFlags::Storage | TextureUsageFlags::Sampled;
			desc.useMsaa = false;
			desc.dedicated = true;
			desc.mipLevels = 1;
			return m_renderingDevice->CreateTexture(desc);
		};

		m_transmittanceLUT = createLut(GetTransmittanceLutSpec());
		m_multiScatteringLUT = createLut(GetMultiScatteringLutSpec());
		m_skyViewLUT = createLut(GetSkyViewLutSpec());
		m_irradianceLUT = createLut(GetIrradianceLutSpec());
		m_prefilteredEnvLUT = createLut(GetPrefilteredEnvLutSpec());
		m_brdfLUT = createLut(GetBrdfLutSpec());
	}

	void Renderer::DestroyAtmosphereLuts()
	{
		auto destroyIfValid = [&](Handle& handle) {
			if (handle.IsValid())
			{
				m_renderingDevice->DestroyTexture(handle);
				handle = {};
			}
		};

		destroyIfValid(m_transmittanceLUT);
		destroyIfValid(m_multiScatteringLUT);
		destroyIfValid(m_skyViewLUT);
		destroyIfValid(m_irradianceLUT);
		destroyIfValid(m_prefilteredEnvLUT);
		destroyIfValid(m_brdfLUT);
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

		auto createTarget = [&](const TextureFormat format, const TextureUsageFlags usage) -> Handle {
			TextureCreateInfo desc{};
			desc.width = width;
			desc.height = height;
			desc.format = format;
			desc.usage = usage;
			desc.useMsaa = false;
			desc.dedicated = true;
			desc.mipLevels = 1;
			return m_renderingDevice->CreateTexture(desc);
		};

		m_depthTarget = createTarget(TextureFormat::D32_SFLOAT, TextureUsageFlags::DepthStencilAttachment | TextureUsageFlags::Sampled | TextureUsageFlags::TransferSrc);
		m_gbufferAlbedo = createTarget(TextureFormat::R8G8B8A8_UNORM, TextureUsageFlags::ColorAttachment | TextureUsageFlags::Sampled);
		m_gbufferNormal = createTarget(TextureFormat::R16G16_SFLOAT, TextureUsageFlags::ColorAttachment | TextureUsageFlags::Sampled | TextureUsageFlags::TransferSrc);
		m_gbufferMaterial = createTarget(TextureFormat::R8G8B8A8_UNORM, TextureUsageFlags::ColorAttachment | TextureUsageFlags::Sampled);
		m_gbufferMotion = createTarget(TextureFormat::R16G16_SFLOAT, TextureUsageFlags::ColorAttachment | TextureUsageFlags::Sampled);
		m_prevDepthTarget = createTarget(TextureFormat::D32_SFLOAT, TextureUsageFlags::TransferDst | TextureUsageFlags::Sampled);
		m_prevNormalTarget = createTarget(TextureFormat::R16G16_SFLOAT, TextureUsageFlags::TransferDst | TextureUsageFlags::Sampled);
		m_hdrColor = createTarget(TextureFormat::R16G16B16A16_SFLOAT, TextureUsageFlags::ColorAttachment | TextureUsageFlags::Storage | TextureUsageFlags::Sampled | TextureUsageFlags::TransferSrc);
		m_taaHistory[0] = createTarget(TextureFormat::R16G16B16A16_SFLOAT, TextureUsageFlags::Storage | TextureUsageFlags::Sampled | TextureUsageFlags::TransferDst);
		m_taaHistory[1] = createTarget(TextureFormat::R16G16B16A16_SFLOAT, TextureUsageFlags::Storage | TextureUsageFlags::Sampled | TextureUsageFlags::TransferDst);
		m_rtShadowRaw = createTarget(TextureFormat::R16G16_SFLOAT, TextureUsageFlags::Storage | TextureUsageFlags::Sampled);
		m_rtShadowPing = createTarget(TextureFormat::R16G16_SFLOAT, TextureUsageFlags::Storage | TextureUsageFlags::Sampled);
		m_rtShadowPong = createTarget(TextureFormat::R16G16_SFLOAT, TextureUsageFlags::Storage | TextureUsageFlags::Sampled);
		m_rtShadowDenoised = createTarget(TextureFormat::R16G16_SFLOAT, TextureUsageFlags::Storage | TextureUsageFlags::Sampled);
		m_rtShadowMoments[0] = createTarget(TextureFormat::R16G16B16A16_SFLOAT, TextureUsageFlags::Storage | TextureUsageFlags::Sampled);
		m_rtShadowMoments[1] = createTarget(TextureFormat::R16G16B16A16_SFLOAT, TextureUsageFlags::Storage | TextureUsageFlags::Sampled);
		m_sssTarget = createTarget(TextureFormat::R16_SFLOAT, TextureUsageFlags::Storage | TextureUsageFlags::Sampled);
		m_aoRaw = createTarget(TextureFormat::R16_SFLOAT, TextureUsageFlags::Storage | TextureUsageFlags::Sampled);
		m_aoDenoised = createTarget(TextureFormat::R16_SFLOAT, TextureUsageFlags::Storage | TextureUsageFlags::Sampled | TextureUsageFlags::TransferSrc);
		m_aoHistory = createTarget(TextureFormat::R16_SFLOAT, TextureUsageFlags::Storage | TextureUsageFlags::Sampled | TextureUsageFlags::TransferDst);

		const uint32_t tileCount = ComputeRTShadowTileCount(width, height);
		const size_t   tileBufferSize = static_cast<size_t>(tileCount) * sizeof(uint32_t);
		BufferDescription tileBufDesc{};
		tileBufDesc.size = tileBufferSize;
		tileBufDesc.bufferUsage = BufferUsageFlags::Storage;
		tileBufDesc.memoryUsage = BufferMemoryUsage::GpuOnly;
		m_rtShadowTileMaskBuffer = m_renderingDevice->CreateBuffer(tileBufDesc);
		m_rtShadowTileMetaBuffer = m_renderingDevice->CreateBuffer(tileBufDesc);

		const uint32_t frameCount = m_renderingDevice->GetMaxFramesInFlight();
		if (m_deferredDescriptorSets.size() != frameCount)
		{
			m_deferredDescriptorSets.resize(frameCount);
			for (uint32_t i = 0; i < frameCount; ++i)
				m_deferredDescriptorSets[i] = m_renderingDevice->CreateDescriptorSet("DeferredLighting");
		}
		for (uint32_t i = 0; i < frameCount; ++i)
		{
			const Handle set = m_deferredDescriptorSets[i];
			m_renderingDevice->UpdateDescriptorCombinedImage(set, 0, m_gbufferAlbedo, m_computeNearestSampler);
			m_renderingDevice->UpdateDescriptorCombinedImage(set, 1, m_gbufferNormal, m_computeNearestSampler);
			m_renderingDevice->UpdateDescriptorCombinedImage(set, 2, m_gbufferMaterial, m_computeNearestSampler);
			m_renderingDevice->UpdateDescriptorCombinedImage(set, 3, m_gbufferMotion, m_computeNearestSampler);
			m_renderingDevice->UpdateDescriptorCombinedImage(set, 4, m_depthTarget, m_computeNearestSampler);
			m_renderingDevice->UpdateDescriptorStorageBuffer(set, 5, m_pointLightBuffer[i], MAX_LIGHT_COUNT * sizeof(PointLightData));
			m_renderingDevice->UpdateDescriptorStorageImage(set, 6, m_hdrColor);
			m_renderingDevice->UpdateDescriptorCombinedImage(set, 7, m_rtShadowDenoised, m_computeNearestSampler);
			m_renderingDevice->UpdateDescriptorCombinedImage(set, 8, m_sssTarget, m_computeNearestSampler);
			m_renderingDevice->UpdateDescriptorCombinedImage(set, 9, m_aoDenoised, m_computeNearestSampler);
		}

		for (uint32_t p = 0; p < 2; ++p)
		{
			if (m_taaDescriptorSets[p].size() != frameCount)
			{
				m_taaDescriptorSets[p].resize(frameCount);
				m_tonemapDescriptorSets[p].resize(frameCount);
				for (uint32_t i = 0; i < frameCount; ++i)
				{
					m_taaDescriptorSets[p][i] = m_renderingDevice->CreateDescriptorSet("PostProcess::TAA");
					m_tonemapDescriptorSets[p][i] = m_renderingDevice->CreateDescriptorSet("PostProcess::ToneMap");
				}
			}
			const uint32_t readIdx = p;
			const uint32_t writeIdx = 1u - p;
			for (uint32_t i = 0; i < frameCount; ++i)
			{
				const Handle taaSet = m_taaDescriptorSets[p][i];
				m_renderingDevice->UpdateDescriptorCombinedImage(taaSet, 0, m_hdrColor, m_linearClampSampler);
				m_renderingDevice->UpdateDescriptorCombinedImage(taaSet, 1, m_taaHistory[readIdx], m_linearClampSampler);
				m_renderingDevice->UpdateDescriptorCombinedImage(taaSet, 2, m_gbufferMotion, m_computeNearestSampler);
				m_renderingDevice->UpdateDescriptorStorageImage(taaSet, 3, m_taaHistory[writeIdx]);

				m_renderingDevice->UpdateDescriptorCombinedImage(m_tonemapDescriptorSets[p][i], 0, m_taaHistory[writeIdx], m_linearClampSampler);
			}
		}

		if (m_rtShadowTraceDescriptorSets.size() != frameCount)
		{
			m_rtShadowTraceDescriptorSets.resize(frameCount);
			for (uint32_t i = 0; i < frameCount; ++i)
				m_rtShadowTraceDescriptorSets[i] = m_renderingDevice->CreateDescriptorSet("RTShadow::Trace");
		}
		for (uint32_t i = 0; i < frameCount; ++i)
		{
			const Handle set = m_rtShadowTraceDescriptorSets[i];
			m_renderingDevice->UpdateDescriptorCombinedImage(set, 0, m_depthTarget, m_computeNearestSampler);
			m_renderingDevice->UpdateDescriptorCombinedImage(set, 1, m_gbufferNormal, m_computeNearestSampler);
			m_renderingDevice->UpdateDescriptorStorageImage(set, 3, m_rtShadowRaw);
			m_renderingDevice->UpdateDescriptorStorageBuffer(set, 4, m_rtShadowTileMaskBuffer, tileBufferSize);
		}

		for (uint32_t p = 0; p < 2; ++p)
		{
			if (m_rtShadowClassifyDescriptorSets[p].size() != frameCount)
			{
				m_rtShadowClassifyDescriptorSets[p].resize(frameCount);
				for (uint32_t i = 0; i < frameCount; ++i)
					m_rtShadowClassifyDescriptorSets[p][i] = m_renderingDevice->CreateDescriptorSet("RTShadow::FFXTileClassification");
			}
			const uint32_t readIdx = p;
			const uint32_t writeIdx = 1u - p;
			for (uint32_t i = 0; i < frameCount; ++i)
			{
				const Handle set = m_rtShadowClassifyDescriptorSets[p][i];
				m_renderingDevice->UpdateDescriptorCombinedImage(set, 0, m_rtShadowRaw, m_computeNearestSampler);
				m_renderingDevice->UpdateDescriptorCombinedImage(set, 1, m_depthTarget, m_computeNearestSampler);
				m_renderingDevice->UpdateDescriptorCombinedImage(set, 2, m_prevDepthTarget, m_linearClampSampler);
				m_renderingDevice->UpdateDescriptorCombinedImage(set, 3, m_gbufferNormal, m_computeNearestSampler);
				m_renderingDevice->UpdateDescriptorCombinedImage(set, 4, m_prevNormalTarget, m_linearClampSampler);
				m_renderingDevice->UpdateDescriptorCombinedImage(set, 5, m_gbufferMotion, m_computeNearestSampler);
				m_renderingDevice->UpdateDescriptorCombinedImage(set, 6, m_rtShadowMoments[readIdx], m_linearClampSampler);
				m_renderingDevice->UpdateDescriptorStorageImage(set, 7, m_rtShadowMoments[writeIdx]);
				m_renderingDevice->UpdateDescriptorStorageImage(set, 8, m_rtShadowPing);
				m_renderingDevice->UpdateDescriptorStorageBuffer(set, 9, m_rtShadowTileMaskBuffer, tileBufferSize);
				m_renderingDevice->UpdateDescriptorStorageBuffer(set, 10, m_rtShadowTileMetaBuffer, tileBufferSize);
			}
		}

		const std::array<Handle, 3> filterInputs = {m_rtShadowPing, m_rtShadowPong, m_rtShadowPing};
		const std::array<Handle, 3> filterOutputs = {m_rtShadowPong, m_rtShadowPing, m_rtShadowDenoised};
		for (uint32_t f = 0; f < 3; ++f)
		{
			if (m_rtShadowFilterDescriptorSets[f].size() != frameCount)
			{
				m_rtShadowFilterDescriptorSets[f].resize(frameCount);
				for (uint32_t i = 0; i < frameCount; ++i)
					m_rtShadowFilterDescriptorSets[f][i] = m_renderingDevice->CreateDescriptorSet("RTShadow::FFXFilterSoftShadows");
			}
			for (uint32_t i = 0; i < frameCount; ++i)
			{
				const Handle set = m_rtShadowFilterDescriptorSets[f][i];
				m_renderingDevice->UpdateDescriptorCombinedImage(set, 0, filterInputs[f], m_computeNearestSampler);
				m_renderingDevice->UpdateDescriptorCombinedImage(set, 1, m_depthTarget, m_computeNearestSampler);
				m_renderingDevice->UpdateDescriptorCombinedImage(set, 2, m_gbufferNormal, m_computeNearestSampler);
				m_renderingDevice->UpdateDescriptorStorageImage(set, 3, filterOutputs[f]);
				m_renderingDevice->UpdateDescriptorStorageBuffer(set, 4, m_rtShadowTileMetaBuffer, tileBufferSize);
			}
		}

		if (m_sssDescriptorSets.size() != frameCount)
		{
			m_sssDescriptorSets.resize(frameCount);
			for (uint32_t i = 0; i < frameCount; ++i)
				m_sssDescriptorSets[i] = m_renderingDevice->CreateDescriptorSet("SSS::Bend");
		}
		for (uint32_t i = 0; i < frameCount; ++i)
		{
			const Handle set = m_sssDescriptorSets[i];
			m_renderingDevice->UpdateDescriptorCombinedImage(set, 0, m_depthTarget, m_pointBorderWhiteSampler);
			m_renderingDevice->UpdateDescriptorStorageImage(set, 1, m_sssTarget);
		}

		if (m_rtaoDescriptorSets.size() != frameCount)
		{
			m_rtaoDescriptorSets.resize(frameCount);
			m_gtaoDescriptorSets.resize(frameCount);
			m_aoDenoiseDescriptorSets.resize(frameCount);
			for (uint32_t i = 0; i < frameCount; ++i)
			{
				m_rtaoDescriptorSets[i] = m_renderingDevice->CreateDescriptorSet("AO::RTAO");
				m_gtaoDescriptorSets[i] = m_renderingDevice->CreateDescriptorSet("AO::GTAO");
				m_aoDenoiseDescriptorSets[i] = m_renderingDevice->CreateDescriptorSet("AO::Denoise");
			}
		}
		for (uint32_t i = 0; i < frameCount; ++i)
		{
			const Handle rtaoSet = m_rtaoDescriptorSets[i];
			m_renderingDevice->UpdateDescriptorCombinedImage(rtaoSet, 0, m_depthTarget, m_computeNearestSampler);
			m_renderingDevice->UpdateDescriptorCombinedImage(rtaoSet, 1, m_gbufferNormal, m_computeNearestSampler);
			m_renderingDevice->UpdateDescriptorStorageImage(rtaoSet, 3, m_aoRaw);

			const Handle gtaoSet = m_gtaoDescriptorSets[i];
			m_renderingDevice->UpdateDescriptorCombinedImage(gtaoSet, 0, m_depthTarget, m_computeNearestSampler);
			m_renderingDevice->UpdateDescriptorCombinedImage(gtaoSet, 1, m_gbufferNormal, m_computeNearestSampler);
			m_renderingDevice->UpdateDescriptorStorageImage(gtaoSet, 2, m_aoRaw);

			const Handle denoiseSet = m_aoDenoiseDescriptorSets[i];
			m_renderingDevice->UpdateDescriptorCombinedImage(denoiseSet, 0, m_aoRaw, m_computeNearestSampler);
			m_renderingDevice->UpdateDescriptorCombinedImage(denoiseSet, 1, m_aoHistory, m_linearClampSampler);
			m_renderingDevice->UpdateDescriptorCombinedImage(denoiseSet, 2, m_depthTarget, m_computeNearestSampler);
			m_renderingDevice->UpdateDescriptorCombinedImage(denoiseSet, 3, m_prevDepthTarget, m_linearClampSampler);
			m_renderingDevice->UpdateDescriptorCombinedImage(denoiseSet, 4, m_gbufferNormal, m_computeNearestSampler);
			m_renderingDevice->UpdateDescriptorCombinedImage(denoiseSet, 5, m_prevNormalTarget, m_linearClampSampler);
			m_renderingDevice->UpdateDescriptorCombinedImage(denoiseSet, 6, m_gbufferMotion, m_computeNearestSampler);
			m_renderingDevice->UpdateDescriptorStorageImage(denoiseSet, 7, m_aoDenoised);
		}

		m_lastSwapChainGeneration = currentGen;
	}

	void Renderer::DestroyRenderTargets()
	{
		auto destroyIfValid = [&](Handle& handle) {
			if (handle.IsValid())
			{
				m_renderingDevice->DestroyTexture(handle);
				handle = {};
			}
		};

		destroyIfValid(m_colorTarget);
		destroyIfValid(m_depthTarget);
		destroyIfValid(m_gbufferAlbedo);
		destroyIfValid(m_gbufferNormal);
		destroyIfValid(m_gbufferMaterial);
		destroyIfValid(m_gbufferMotion);
		destroyIfValid(m_prevDepthTarget);
		destroyIfValid(m_prevNormalTarget);
		destroyIfValid(m_hdrColor);
		destroyIfValid(m_taaHistory[0]);
		destroyIfValid(m_taaHistory[1]);
		destroyIfValid(m_rtShadowRaw);
		destroyIfValid(m_rtShadowPing);
		destroyIfValid(m_rtShadowPong);
		destroyIfValid(m_rtShadowDenoised);
		destroyIfValid(m_rtShadowMoments[0]);
		destroyIfValid(m_rtShadowMoments[1]);
		destroyIfValid(m_sssTarget);
		destroyIfValid(m_aoRaw);
		destroyIfValid(m_aoDenoised);
		destroyIfValid(m_aoHistory);

		auto destroyBufferIfValid = [&](Handle& handle) {
			if (handle.IsValid())
			{
				m_renderingDevice->DestroyBuffer(handle);
				handle = {};
			}
		};
		destroyBufferIfValid(m_rtShadowTileMaskBuffer);
		destroyBufferIfValid(m_rtShadowTileMetaBuffer);
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
		ctx.gbufferAlbedo = m_gbufferAlbedo;
		ctx.gbufferNormal = m_gbufferNormal;
		ctx.gbufferMaterial = m_gbufferMaterial;
		ctx.gbufferMotion = m_gbufferMotion;
		ctx.prevDepthTarget = m_prevDepthTarget;
		const uint32_t readIdx = m_temporalReadIndex;
		const uint32_t writeIdx = 1u - readIdx;

		ctx.prevNormalTarget = m_prevNormalTarget;
		ctx.hdrColor = m_hdrColor;
		ctx.taaHistoryRead = m_taaHistory[readIdx];
		ctx.taaResolved = m_taaHistory[writeIdx];
		ctx.rtShadowRaw = m_rtShadowRaw;
		ctx.rtShadowPing = m_rtShadowPing;
		ctx.rtShadowPong = m_rtShadowPong;
		ctx.rtShadowDenoised = m_rtShadowDenoised;
		ctx.rtShadowMomentsRead = m_rtShadowMoments[readIdx];
		ctx.rtShadowMomentsWrite = m_rtShadowMoments[writeIdx];
		ctx.sssTarget = m_sssTarget;
		ctx.aoRaw = m_aoRaw;
		ctx.aoDenoised = m_aoDenoised;
		ctx.aoHistory = m_aoHistory;
		ctx.rtShadowTileMaskBuffer = m_rtShadowTileMaskBuffer;
		ctx.rtShadowTileMetaBuffer = m_rtShadowTileMetaBuffer;
		ctx.transmittanceLUT = m_transmittanceLUT;
		ctx.multiScatteringLUT = m_multiScatteringLUT;
		ctx.skyViewLUT = m_skyViewLUT;
		ctx.irradianceLUT = m_irradianceLUT;
		ctx.prefilteredEnvLUT = m_prefilteredEnvLUT;
		ctx.brdfLUT = m_brdfLUT;
		ctx.globalDescriptorSet = m_globalDescriptorSets[frameIndex];
		ctx.deferredLightingDescriptorSet = m_deferredDescriptorSets[frameIndex];
		ctx.taaDescriptorSet = m_taaDescriptorSets[readIdx][frameIndex];
		ctx.tonemapDescriptorSet = m_tonemapDescriptorSets[readIdx][frameIndex];
		ctx.rtShadowTraceDescriptorSet = m_rtShadowTraceDescriptorSets[frameIndex];
		ctx.rtShadowClassifyDescriptorSet = m_rtShadowClassifyDescriptorSets[readIdx][frameIndex];
		for (uint32_t f = 0; f < 3; ++f)
			ctx.rtShadowFilterDescriptorSets[f] = m_rtShadowFilterDescriptorSets[f][frameIndex];
		ctx.sssDescriptorSet = m_sssDescriptorSets[frameIndex];
		ctx.rtaoDescriptorSet = m_rtaoDescriptorSets[frameIndex];
		ctx.gtaoDescriptorSet = m_gtaoDescriptorSets[frameIndex];
		ctx.aoDenoiseDescriptorSet = m_aoDenoiseDescriptorSets[frameIndex];
		ctx.unjitteredViewProj = m_prevUnjitteredViewProj;
		ctx.lightDirection = m_lightDirection;
		ctx.opaqueStream = m_opaqueDrawStream.get();
		ctx.transparentStream = m_transparentDrawStream.get();

		for (const auto& pass : m_passes)
			pass->Execute(ctx);

		if (m_depthTarget.IsValid() && m_prevDepthTarget.IsValid())
			m_renderingDevice->CopyTexture(m_depthTarget, m_prevDepthTarget);
		if (m_gbufferNormal.IsValid() && m_prevNormalTarget.IsValid())
			m_renderingDevice->CopyTexture(m_gbufferNormal, m_prevNormalTarget);

		m_temporalReadIndex = 1u - m_temporalReadIndex;

		const auto transformView = scene->GetView<TransformComponent>();
		for (auto [entity, transform] : transformView.each())
			transform.prevWorldMatrix = transform.worldMatrix;

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

		const auto&	   settings = RenderSettings::Get();
		const auto [swapW, swapH] = m_renderingDevice->GetSwapChainExtent();

		const Matrix4& viewMatrix = camera->GetViewMatrix();
		const Matrix4  projMatrix = camera->GetProjectionMatrix(aspectRatio);
		const Matrix4  unjitteredViewProj = projMatrix * viewMatrix;
		Matrix4		   jitteredProj = projMatrix;
		if (settings.enableTAA && settings.debugView == EDebugViewMode::Final && swapW > 0 && swapH > 0)
		{
			const uint32_t sampleIdx = (m_frameIndex % 16u) + 1u;
			const float	   jitterX = (Halton(sampleIdx, 2u) - 0.5f) * (2.0f / static_cast<float>(swapW));
			const float	   jitterY = (Halton(sampleIdx, 3u) - 0.5f) * (2.0f / static_cast<float>(swapH));
			jitteredProj = Matrix4::Identity.Translate(jitterX, jitterY, 0.0f) * projMatrix;
		}
		const Matrix4 viewProj = jitteredProj * viewMatrix;
		const Frustum frustum = Frustum::FromViewProjection(viewProj);

		const auto globalUniform = GetGlobalUniformBuffer(frameIndex);
		globalUniform->view = viewMatrix;
		globalUniform->projection = jitteredProj;
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
		globalUniform->debugView = static_cast<uint32_t>(settings.debugView);
		globalUniform->toneMapMode = static_cast<uint32_t>(settings.toneMapMode);
		uint32_t flags = settings.enableTAA ? 1u : 0u;
		if (settings.enableScreenSpaceShadows)
			flags |= (1u << 2);
		if (settings.aoMode != EAOMode::Off)
			flags |= (1u << 3);
		globalUniform->flags = flags;

		m_lightDirection = dirLightDirection;
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
