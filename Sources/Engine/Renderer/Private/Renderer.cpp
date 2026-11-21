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

#include "Passes/AmbientOcclusionPass.hpp"
#include "Passes/DeferredLightingPass.hpp"
#include "Passes/EditorPass.hpp"
#include "Passes/ForwardTransparentPass.hpp"
#include "Passes/GBufferPass.hpp"
#include "Passes/PostProcessPass.hpp"
#include "Passes/RTShadowPass.hpp"
#include "Passes/ScreenSpaceShadowPass.hpp"
#include "Passes/SkyAtmospherePass.hpp"
#include "Passes/VolumetricFogPass.hpp"
#include "Passes/WaterPass.hpp"

#include <Common/Logger.hpp>
#include <Common/Metrics.hpp>
#include <Common/RenderSettings.hpp>
#include <Data/Graphics/DynamicUniformData.hpp>
#include <Data/Graphics/GlobalUniformData.hpp>
#include <Data/Graphics/VertexData.hpp>
#include <Math/Frustum.hpp>
#include <RenderingDevice/Blueprint/PipelineLayoutBlueprint.hpp>
#include <RenderingDevice/BufferDescription.hpp>
#include <RenderingDevice/DrawStream.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <RenderingDevice/SamplerDescription.hpp>
#include <RenderingDevice/TextureDescription.hpp>
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

	struct FfxShadowFilterPushLayout
	{
		int32_t stepSize = 1;
		float	depthSigma = 0.015f;
		float	normalSigma = 16.0f;
		float	shadowPhi = 4.0f;
	};

	static float Halton(uint32_t index, const uint32_t base)
	{
		float result = 0.0f;
		float f = 1.0f / static_cast<float>(base);
		while (index > 0)
		{
			result += f * static_cast<float>(index % base);
			index /= base;
			f /= static_cast<float>(base);
		}
		return result;
	}

	Renderer::Renderer(RenderingDevice* renderingDevice, ResourceManager* resourceManager, Editor* editor)
		: m_renderingDevice(renderingDevice), m_resourceManager(resourceManager), m_editor(editor)
	{
		m_rtSupported = m_renderingDevice->IsRayQuerySupported();
		RenderSettings::Get().rtSupported = m_rtSupported;
		if (!m_rtSupported && RenderSettings::Get().aoMode == EAOMode::RTAO)
			RenderSettings::Get().aoMode = EAOMode::GTAO;

		SamplerDescription nearestSamplerDesc{};
		nearestSamplerDesc.magFilter = SamplerFilter::Nearest;
		nearestSamplerDesc.minFilter = SamplerFilter::Nearest;
		nearestSamplerDesc.addressModeU = SamplerAddressMode::ClampToEdge;
		nearestSamplerDesc.addressModeV = SamplerAddressMode::ClampToEdge;
		nearestSamplerDesc.addressModeW = SamplerAddressMode::ClampToEdge;
		nearestSamplerDesc.anisotropyEnable = false;
		nearestSamplerDesc.mipmapMode = SamplerMipmapMode::Nearest;
		m_nearestClampSampler = m_renderingDevice->GetOrCreateTextureSampler(nearestSamplerDesc);

		SamplerDescription nearestRepeatDesc = nearestSamplerDesc;
		nearestRepeatDesc.addressModeU = SamplerAddressMode::Repeat;
		nearestRepeatDesc.addressModeV = SamplerAddressMode::Repeat;
		nearestRepeatDesc.addressModeW = SamplerAddressMode::Repeat;
		m_nearestRepeatSampler = m_renderingDevice->GetOrCreateTextureSampler(nearestRepeatDesc);

		SamplerDescription pointBorderWhiteDesc{};
		pointBorderWhiteDesc.magFilter = SamplerFilter::Nearest;
		pointBorderWhiteDesc.minFilter = SamplerFilter::Nearest;
		pointBorderWhiteDesc.addressModeU = SamplerAddressMode::ClampToBorder;
		pointBorderWhiteDesc.addressModeV = SamplerAddressMode::ClampToBorder;
		pointBorderWhiteDesc.addressModeW = SamplerAddressMode::ClampToBorder;
		pointBorderWhiteDesc.borderColor = BorderColor::FloatOpaqueWhite;
		pointBorderWhiteDesc.anisotropyEnable = false;
		pointBorderWhiteDesc.mipmapMode = SamplerMipmapMode::Nearest;
		m_pointBorderWhiteSampler = m_renderingDevice->GetOrCreateTextureSampler(pointBorderWhiteDesc);

		SamplerDescription linearSamplerDesc{};
		linearSamplerDesc.magFilter = SamplerFilter::Linear;
		linearSamplerDesc.minFilter = SamplerFilter::Linear;
		linearSamplerDesc.addressModeU = SamplerAddressMode::ClampToEdge;
		linearSamplerDesc.addressModeV = SamplerAddressMode::ClampToEdge;
		linearSamplerDesc.addressModeW = SamplerAddressMode::ClampToEdge;
		linearSamplerDesc.anisotropyEnable = false;
		linearSamplerDesc.mipmapMode = SamplerMipmapMode::Nearest;
		m_linearClampSampler = m_renderingDevice->GetOrCreateTextureSampler(linearSamplerDesc);

		SamplerDescription skyViewSamplerDesc{};
		skyViewSamplerDesc.magFilter = SamplerFilter::Linear;
		skyViewSamplerDesc.minFilter = SamplerFilter::Linear;
		skyViewSamplerDesc.addressModeU = SamplerAddressMode::Repeat;
		skyViewSamplerDesc.addressModeV = SamplerAddressMode::ClampToEdge;
		skyViewSamplerDesc.addressModeW = SamplerAddressMode::ClampToEdge;
		skyViewSamplerDesc.anisotropyEnable = false;
		skyViewSamplerDesc.mipmapMode = SamplerMipmapMode::Nearest;
		m_skyViewSampler = m_renderingDevice->GetOrCreateTextureSampler(skyViewSamplerDesc);

		CreateLayoutsAndPipelines();

		m_opaqueDrawStream = std::make_unique<DrawStream>();
		m_waterDrawStream = std::make_unique<DrawStream>();
		m_transparentDrawStream = std::make_unique<DrawStream>();

		const uint32_t frameCount = m_renderingDevice->GetMaxFramesInFlight();
		m_tlasHandles.resize(frameCount);
		m_tlasScratchBuffers.resize(frameCount);
		m_tlasInstanceBuffers.resize(frameCount);
		m_tlasBoundToSets.assign(frameCount, false);

		CreateGlobalUniformBuffers();
		CreateGlobalUniformDescriptorSets();
		CreatePointLightBuffers();
		CreatePointLightDescriptorSets();

		// Allocate persistent descriptor sets
		if (m_rtSupported)
		{
			m_rtShadowDescriptorSets.resize(frameCount);
			m_rtaoDescriptorSets.resize(frameCount);
			m_volumetricFogRtDescriptorSets.resize(frameCount);
			for (uint32_t i = 0; i < frameCount; ++i)
			{
				m_rtShadowDescriptorSets[i] = m_renderingDevice->CreateDescriptorSet("RTShadow");
				m_rtaoDescriptorSets[i] = m_renderingDevice->CreateDescriptorSet("RTAO");
				m_volumetricFogRtDescriptorSets[i] = m_renderingDevice->CreateDescriptorSet("VolumetricFogRT");
			}
		}
		for (uint32_t i = 0; i < 2; ++i)
			m_ffxTileClassDescriptorSets[i] = m_renderingDevice->CreateDescriptorSet("FFXShadowTileClass");
		for (uint32_t i = 0; i < 3; ++i)
			m_ffxFilterDescriptorSets[i] = m_renderingDevice->CreateDescriptorSet("FFXShadowFilter");

		m_bendSssDescriptorSet = m_renderingDevice->CreateDescriptorSet("BendSSS");
		m_gtaoDescriptorSet = m_renderingDevice->CreateDescriptorSet("GTAO");
		m_aoDenoiseDescriptorSet = m_renderingDevice->CreateDescriptorSet("AODenoise");
		for (uint32_t i = 0; i < 3; ++i)
			m_aoFilterDescriptorSets[i] = m_renderingDevice->CreateDescriptorSet("AODenoise");

		m_skyTransmittanceDescriptorSet = m_renderingDevice->CreateDescriptorSet("SkyTransmittance");
		m_skyMultiScatteringDescriptorSet = m_renderingDevice->CreateDescriptorSet("SkyMultiScattering");
		m_skyViewDescriptorSet = m_renderingDevice->CreateDescriptorSet("SkyView");
		m_skyIrradianceDescriptorSet = m_renderingDevice->CreateDescriptorSet("SkyIrradiance");
		m_skyPrefilterDescriptorSet = m_renderingDevice->CreateDescriptorSet("SkyPrefilter");
		m_skyBrdfDescriptorSet = m_renderingDevice->CreateDescriptorSet("SkyBrdf");
		m_skyRenderDescriptorSet = m_renderingDevice->CreateDescriptorSet("SkyRender");

		m_deferredLightingDescriptorSet = m_renderingDevice->CreateDescriptorSet("DeferredLighting");
		m_waterDescriptorSet = m_renderingDevice->CreateDescriptorSet("Water");
		m_volumetricFogNoRtDescriptorSet = m_renderingDevice->CreateDescriptorSet("VolumetricFogNoRT");
		m_forwardIblDescriptorSet = m_renderingDevice->CreateDescriptorSet("ForwardIBL");
		for (uint32_t i = 0; i < 2; ++i)
		{
			m_taaDescriptorSets[i] = m_renderingDevice->CreateDescriptorSet("TAA");
			m_toneMapDescriptorSets[i] = m_renderingDevice->CreateDescriptorSet("ToneMap");
		}
		m_fgToneMapDescriptorSet = m_renderingDevice->CreateDescriptorSet("ToneMap");

		CreateBlueNoiseTexture();
		CreateAtmosphereLuts();
		EnsureRenderTargets();

		AddPass(std::make_unique<GBufferPass>());
		AddPass(std::make_unique<RTShadowPass>());
		AddPass(std::make_unique<ScreenSpaceShadowPass>());
		AddPass(std::make_unique<AmbientOcclusionPass>());
		AddPass(std::make_unique<SkyAtmospherePass>());
		AddPass(std::make_unique<DeferredLightingPass>());
		AddPass(std::make_unique<WaterPass>());
		AddPass(std::make_unique<ForwardTransparentPass>());
		AddPass(std::make_unique<VolumetricFogPass>());
		AddPass(std::make_unique<PostProcessPass>());
#ifndef NDEBUG
		if (m_editor)
			AddPass(std::make_unique<EditorPass>());
#endif

		Logger::Log<SUCCESS>("Renderer: deferred + FFX RT shadows + Bend SSS + AO + Hillaire sky + Water + Volumetric Fog initialized ({} passes, HW Ray Query: {})", m_passes.size(), m_rtSupported ? "enabled" : "unavailable");
	}

	Renderer::~Renderer()
	{
		if (!m_renderingDevice)
			return;

		m_renderingDevice->WaitIdle();

		DestroyRenderTargets();

		auto destroyTex = [&](Handle& handle) {
			if (handle.IsValid())
			{
				m_renderingDevice->DestroyTexture(handle);
				handle = {};
			}
		};
		destroyTex(m_transmittanceLUT);
		destroyTex(m_multiScatteringLUT);
		destroyTex(m_skyViewLUT);
		destroyTex(m_irradianceLUT);
		destroyTex(m_prefilteredEnvLUT);
		destroyTex(m_brdfLUT);
		destroyTex(m_blueNoiseTexture);

		for (const auto& [meshId, blasHandle] : m_blasCache)
		{
			if (blasHandle.IsValid())
				m_renderingDevice->DestroyAccelerationStructure(blasHandle);
		}
		m_blasCache.clear();

		for (const Handle& handle : m_tlasHandles)
		{
			if (handle.IsValid())
				m_renderingDevice->DestroyAccelerationStructure(handle);
		}
		for (const Handle& handle : m_tlasScratchBuffers)
		{
			if (handle.IsValid())
				m_renderingDevice->DestroyBuffer(handle);
		}
		for (const Handle& handle : m_tlasInstanceBuffers)
		{
			if (handle.IsValid())
				m_renderingDevice->DestroyBuffer(handle);
		}

		auto destroySet = [&](Handle& handle) {
			if (handle.IsValid())
			{
				m_renderingDevice->DestroyDescriptorSet(handle);
				handle = {};
			}
		};

		for (Handle& h : m_rtShadowDescriptorSets)
			destroySet(h);
		for (Handle& h : m_rtaoDescriptorSets)
			destroySet(h);
		for (Handle& h : m_volumetricFogRtDescriptorSets)
			destroySet(h);
		for (Handle& h : m_ffxTileClassDescriptorSets)
			destroySet(h);
		for (Handle& h : m_ffxFilterDescriptorSets)
			destroySet(h);
		destroySet(m_bendSssDescriptorSet);
		destroySet(m_gtaoDescriptorSet);
		destroySet(m_aoDenoiseDescriptorSet);
		for (Handle& h : m_aoFilterDescriptorSets)
			destroySet(h);
		destroySet(m_skyTransmittanceDescriptorSet);
		destroySet(m_skyMultiScatteringDescriptorSet);
		destroySet(m_skyViewDescriptorSet);
		destroySet(m_skyIrradianceDescriptorSet);
		destroySet(m_skyPrefilterDescriptorSet);
		destroySet(m_skyBrdfDescriptorSet);
		destroySet(m_skyRenderDescriptorSet);
		destroySet(m_deferredLightingDescriptorSet);
		destroySet(m_waterDescriptorSet);
		destroySet(m_volumetricFogNoRtDescriptorSet);
		destroySet(m_forwardIblDescriptorSet);
		for (Handle& h : m_taaDescriptorSets)
			destroySet(h);
		for (Handle& h : m_toneMapDescriptorSets)
			destroySet(h);
		destroySet(m_fgToneMapDescriptorSet);

		for (Handle& h : m_globalDescriptorSets)
			destroySet(h);
		for (const Handle& h : m_globalUniformBuffer)
		{
			if (h.IsValid())
				m_renderingDevice->DestroyBuffer(h);
		}
		for (Handle& h : m_pointLightDescriptorSets)
			destroySet(h);
		for (const Handle& h : m_pointLightBuffer)
		{
			if (h.IsValid())
				m_renderingDevice->DestroyBuffer(h);
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

	void Renderer::CreateLayoutsAndPipelines()
	{
		// Set 0: Global per-frame uniforms
		m_renderingDevice->CreateDescriptorSetLayout("GlobalUniforms", DescriptorType::UniformBuffer, ShaderStageFlags::Vertex | ShaderStageFlags::Fragment | ShaderStageFlags::Compute);

		// Set 1: PBR Material
		const std::array<DescriptorBindingDesc, 4> materialBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Fragment},
			{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Fragment},
			{.binding = 2, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Fragment},
			{.binding = 3, .type = DescriptorType::UniformBuffer, .count = 1, .stages = ShaderStageFlags::Fragment},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("Material", materialBindings);

		// Set 1 (Terrain): 5 Ground PBR layers x (Color, NormalGL, Roughness) = 15 samplers
		std::array<DescriptorBindingDesc, 15> terrainMaterialBindings{};
		for (uint32_t i = 0; i < 15u; ++i)
			terrainMaterialBindings[i] = {.binding = i, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Fragment};
		m_renderingDevice->CreateDescriptorSetLayout("TerrainMaterial", terrainMaterialBindings);

		// Set 1 (Water): depth, refracted scene HDR, transmittance, irradiance, prefilteredEnv, brdfLUT
		const std::array<DescriptorBindingDesc, 6> waterBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Fragment},
			{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Fragment},
			{.binding = 2, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Fragment},
			{.binding = 3, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Fragment},
			{.binding = 4, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Fragment},
			{.binding = 5, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Fragment},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("Water", waterBindings);

		// Set 2: Point lights SSBO
		m_renderingDevice->CreateDescriptorSetLayout("PointLights", DescriptorType::StorageBuffer, ShaderStageFlags::Fragment | ShaderStageFlags::Compute);

		// Forward IBL (Set 3 for ForwardTransparentPass)
		const std::array<DescriptorBindingDesc, 4> forwardIblBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Fragment},
			{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Fragment},
			{.binding = 2, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Fragment},
			{.binding = 3, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Fragment},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("ForwardIBL", forwardIblBindings);

		// RTShadow trace + FFX 8x4 bitmask packing
		if (m_rtSupported)
		{
			const std::array<DescriptorBindingDesc, 5> rtShadowBindings = {{
				{.binding = 0, .type = DescriptorType::AccelerationStructure, .count = 1, .stages = ShaderStageFlags::Compute},
				{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
				{.binding = 2, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
				{.binding = 3, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
				{.binding = 4, .type = DescriptorType::StorageBuffer, .count = 1, .stages = ShaderStageFlags::Compute},
			}};
			m_renderingDevice->CreateDescriptorSetLayout("RTShadow", rtShadowBindings);

			const std::array<DescriptorBindingDesc, 5> rtaoBindings = {{
				{.binding = 0, .type = DescriptorType::AccelerationStructure, .count = 1, .stages = ShaderStageFlags::Compute},
				{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
				{.binding = 2, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
				{.binding = 3, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
				{.binding = 4, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			}};
			m_renderingDevice->CreateDescriptorSetLayout("RTAO", rtaoBindings);

			const std::array<DescriptorBindingDesc, 6> volumetricFogRtBindings = {{
				{.binding = 0, .type = DescriptorType::AccelerationStructure, .count = 1, .stages = ShaderStageFlags::Compute},
				{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
				{.binding = 2, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
				{.binding = 3, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
				{.binding = 4, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
				{.binding = 5, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
			}};
			m_renderingDevice->CreateDescriptorSetLayout("VolumetricFogRT", volumetricFogRtBindings);
		}

		const std::array<DescriptorBindingDesc, 5> volumetricFogNoRtBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 2, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 3, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 4, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("VolumetricFogNoRT", volumetricFogNoRtBindings);

		// AMD FidelityFX Shadow Denoiser - Stage 1: Tile Classification + Temporal Moments
		const std::array<DescriptorBindingDesc, 11> ffxTileClassBindings = {{
			{.binding = 0, .type = DescriptorType::StorageBuffer, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 1, .type = DescriptorType::StorageBuffer, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 2, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 3, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 4, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 5, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 6, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 7, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 8, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 9, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 10, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("FFXShadowTileClass", ffxTileClassBindings);

		// AMD FidelityFX Shadow Denoiser - Stage 2: A-Trous Soft Shadow Filter
		const std::array<DescriptorBindingDesc, 5> ffxFilterBindings = {{
			{.binding = 0, .type = DescriptorType::StorageBuffer, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 2, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 3, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 4, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("FFXShadowFilter", ffxFilterBindings);

		// Bend Studio Screen Space Shadows (depth sampler + output storage image)
		const std::array<DescriptorBindingDesc, 2> bendSssBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 1, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("BendSSS", bendSssBindings);

		// Ambient Occlusion (GTAO & Denoise)
		const std::array<DescriptorBindingDesc, 4> gtaoBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 2, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 3, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("GTAO", gtaoBindings);

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
		m_renderingDevice->CreateDescriptorSetLayout("AODenoise", aoDenoiseBindings);

		// Hillaire Sky & Irradiance / IBL LUT layouts
		m_renderingDevice->CreateDescriptorSetLayout("SkyTransmittance", DescriptorType::StorageImage, ShaderStageFlags::Compute);
		m_renderingDevice->CreateDescriptorSetLayout("SkyBrdf", DescriptorType::StorageImage, ShaderStageFlags::Compute);

		const std::array<DescriptorBindingDesc, 2> skyMsBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 1, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("SkyMultiScattering", skyMsBindings);
		m_renderingDevice->CreateDescriptorSetLayout("SkyIrradiance", skyMsBindings);
		m_renderingDevice->CreateDescriptorSetLayout("SkyPrefilter", skyMsBindings);

		const std::array<DescriptorBindingDesc, 3> skyViewBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 2, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("SkyView", skyViewBindings);

		const std::array<DescriptorBindingDesc, 5> skyRenderBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 2, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 3, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 4, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("SkyRender", skyRenderBindings);

		// DeferredLighting set (G-Buffer + FFX RT Shadows + HDR out + Transmittance + Irradiance + Prefiltered Env + BRDF LUT + AO + Bend SSS)
		const std::array<DescriptorBindingDesc, 13> deferredLightingBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 2, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 3, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 4, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 5, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 6, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 7, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 8, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 9, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 10, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 11, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 12, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("DeferredLighting", deferredLightingBindings);

		// TAA set
		const std::array<DescriptorBindingDesc, 5> taaBindings = {{
			{.binding = 0, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 1, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 2, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 3, .type = DescriptorType::CombinedImageSampler, .count = 1, .stages = ShaderStageFlags::Compute},
			{.binding = 4, .type = DescriptorType::StorageImage, .count = 1, .stages = ShaderStageFlags::Compute},
		}};
		m_renderingDevice->CreateDescriptorSetLayout("TAA", taaBindings);

		// ToneMap set
		m_renderingDevice->CreateDescriptorSetLayout("ToneMap", DescriptorType::CombinedImageSampler, ShaderStageFlags::Fragment);

		// Pipeline layouts
		m_renderingDevice->CreatePipelineLayout("GBuffer", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("Material");
			blueprint->AddPushConstant<DynamicUniformData>(EShaderType::Vertex);
		});

		m_renderingDevice->CreatePipelineLayout("GBufferTerrain", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("TerrainMaterial");
			blueprint->AddPushConstant<DynamicUniformData>(EShaderType::Vertex);
		});

		m_renderingDevice->CreatePipelineLayout("Water", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("Water");
			blueprint->AddPushConstant<DynamicUniformData>(EShaderType::Vertex);
		});

		m_renderingDevice->CreatePipelineLayout("Forward", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("Material");
			blueprint->AddDescriptorSetLayout("PointLights");
			blueprint->AddDescriptorSetLayout("ForwardIBL");
			blueprint->AddPushConstant<DynamicUniformData>(EShaderType::Vertex);
		});

		if (m_rtSupported)
		{
			m_renderingDevice->CreatePipelineLayout("RTShadow", [&](PipelineLayoutBlueprint* blueprint) {
				blueprint->AddDescriptorSetLayout("GlobalUniforms");
				blueprint->AddDescriptorSetLayout("RTShadow");
			});
			m_renderingDevice->CreatePipelineLayout("RTAO", [&](PipelineLayoutBlueprint* blueprint) {
				blueprint->AddDescriptorSetLayout("GlobalUniforms");
				blueprint->AddDescriptorSetLayout("RTAO");
			});
			m_renderingDevice->CreatePipelineLayout("VolumetricFogRT", [&](PipelineLayoutBlueprint* blueprint) {
				blueprint->AddDescriptorSetLayout("GlobalUniforms");
				blueprint->AddDescriptorSetLayout("VolumetricFogRT");
				blueprint->AddPushConstant<VolumetricFogPushConstants>(EShaderType::Compute);
			});
		}

		m_renderingDevice->CreatePipelineLayout("VolumetricFogNoRT", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("VolumetricFogNoRT");
			blueprint->AddPushConstant<VolumetricFogPushConstants>(EShaderType::Compute);
		});

		// Forward declarations of push constant layouts for pipeline creation
		struct FfxShadowTileClassPushLayout
		{
			float stabilityBias = 0.08f;
			float radianceClipStdK = 1.5f;
			float disocclusionThreshold = 0.025f;
			float _pad = 0.0f;
		};

		m_renderingDevice->CreatePipelineLayout("FFXShadowTileClass", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("FFXShadowTileClass");
			blueprint->AddPushConstant<FfxShadowTileClassPushLayout>(EShaderType::Compute);
		});

		m_renderingDevice->CreatePipelineLayout("FFXShadowFilter", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("FFXShadowFilter");
			blueprint->AddPushConstant<FfxShadowFilterPushLayout>(EShaderType::Compute);
		});

		m_renderingDevice->CreatePipelineLayout("BendSSS", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("BendSSS");
			blueprint->AddPushConstant<BendSssPushConstants>(EShaderType::Compute);
		});

		m_renderingDevice->CreatePipelineLayout("GTAO", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("GTAO");
		});

		struct AoDenoisePushLayout
		{
			int32_t stepSize = 0;
			float	depthSigma = 0.015f;
			float	normalSigma = 16.0f;
			float	variancePhi = 4.0f;
			float	stabilityBias = 0.08f;
			float	radianceClipStdK = 1.5f;
			float	disocclusionThreshold = 0.025f;
			float	_pad = 0.0f;
		};

		m_renderingDevice->CreatePipelineLayout("AODenoise", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("AODenoise");
			blueprint->AddPushConstant<AoDenoisePushLayout>(EShaderType::Compute);
		});

		m_renderingDevice->CreatePipelineLayout("SkyTransmittance", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("SkyTransmittance");
		});

		m_renderingDevice->CreatePipelineLayout("SkyMultiScattering", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("SkyMultiScattering");
		});

		m_renderingDevice->CreatePipelineLayout("SkyView", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("SkyView");
		});

		m_renderingDevice->CreatePipelineLayout("SkyIrradiance", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("SkyIrradiance");
		});

		m_renderingDevice->CreatePipelineLayout("SkyPrefilter", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("SkyPrefilter");
		});

		m_renderingDevice->CreatePipelineLayout("SkyBrdf", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("SkyBrdf");
		});

		m_renderingDevice->CreatePipelineLayout("SkyRender", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("SkyRender");
		});

		m_renderingDevice->CreatePipelineLayout("DeferredLighting", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("DeferredLighting");
			blueprint->AddDescriptorSetLayout("PointLights");
		});

		m_renderingDevice->CreatePipelineLayout("TAA", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("TAA");
		});

		m_renderingDevice->CreatePipelineLayout("ToneMap", [&](PipelineLayoutBlueprint* blueprint) {
			blueprint->AddDescriptorSetLayout("GlobalUniforms");
			blueprint->AddDescriptorSetLayout("ToneMap");
		});

		const VertexLayout vertexLayout = {VertexType::Float3, VertexType::Float3, VertexType::Float2, VertexType::Float3};

		// G-Buffer Opaque pipeline
		const auto gbufferVert = m_resourceManager->LoadShader("Shaders/deferred/gbuffer.vert");
		const auto gbufferFrag = m_resourceManager->LoadShader("Shaders/deferred/gbuffer.frag");
		m_resourceManager->CreatePipeline("GBuffer::Opaque", [&](PipelineBlueprint* blueprint) {
			blueprint->SetVertexLayout(vertexLayout)
				->AddShader(gbufferVert)
				->AddShader(gbufferFrag)
				->SetPipelineLayout("GBuffer")
				->SetColorFormats({
					TextureFormat::R8G8B8A8_UNORM,
					TextureFormat::R16G16_SFLOAT,
					TextureFormat::R8G8B8A8_UNORM,
					TextureFormat::R16G16_SFLOAT,
				})
				->SetHasDepthAttachment(true)
				->SetUseMsaa(false)
				->SetDepthTest(true)
				->SetDepthWrite(true)
				->SetBlendEnable(false)
				->SetAlphaToCoverage(false);
		});

		// G-Buffer Masked double-sided pipeline (foliage, elm tree leaves, curtains)
		m_resourceManager->CreatePipeline("GBuffer::Masked", [&](PipelineBlueprint* blueprint) {
			blueprint->SetVertexLayout(vertexLayout)
				->AddShader(gbufferVert)
				->AddShader(gbufferFrag)
				->SetPipelineLayout("GBuffer")
				->SetColorFormats({
					TextureFormat::R8G8B8A8_UNORM,
					TextureFormat::R16G16_SFLOAT,
					TextureFormat::R8G8B8A8_UNORM,
					TextureFormat::R16G16_SFLOAT,
				})
				->SetHasDepthAttachment(true)
				->SetUseMsaa(false)
				->SetDepthTest(true)
				->SetDepthWrite(true)
				->SetBlendEnable(false)
				->SetAlphaToCoverage(false)
				->SetCullMode(CullMode::None);
		});

		// G-Buffer Multi-Layer PBR Terrain pipeline
		const auto terrainFrag = m_resourceManager->LoadShader("Shaders/deferred/terrain_gbuffer.frag");
		m_resourceManager->CreatePipeline("GBuffer::Terrain", [&](PipelineBlueprint* blueprint) {
			blueprint->SetVertexLayout(vertexLayout)
				->AddShader(gbufferVert)
				->AddShader(terrainFrag)
				->SetPipelineLayout("GBufferTerrain")
				->SetColorFormats({
					TextureFormat::R8G8B8A8_UNORM,
					TextureFormat::R16G16_SFLOAT,
					TextureFormat::R8G8B8A8_UNORM,
					TextureFormat::R16G16_SFLOAT,
				})
				->SetHasDepthAttachment(true)
				->SetUseMsaa(false)
				->SetDepthTest(true)
				->SetDepthWrite(true)
				->SetBlendEnable(false)
				->SetAlphaToCoverage(false);
		});

		// G-Buffer Procedural 3D Grass pipeline (animated wind + double-sided curved blades)
		const auto grassVert = m_resourceManager->LoadShader("Shaders/deferred/grass_gbuffer.vert");
		const auto grassFrag = m_resourceManager->LoadShader("Shaders/deferred/grass_gbuffer.frag");
		m_resourceManager->CreatePipeline("GBuffer::Grass", [&](PipelineBlueprint* blueprint) {
			blueprint->SetVertexLayout(vertexLayout)
				->AddShader(grassVert)
				->AddShader(grassFrag)
				->SetPipelineLayout("GBuffer")
				->SetColorFormats({
					TextureFormat::R8G8B8A8_UNORM,
					TextureFormat::R16G16_SFLOAT,
					TextureFormat::R8G8B8A8_UNORM,
					TextureFormat::R16G16_SFLOAT,
				})
				->SetHasDepthAttachment(true)
				->SetUseMsaa(false)
				->SetDepthTest(true)
				->SetDepthWrite(true)
				->SetBlendEnable(false)
				->SetAlphaToCoverage(false)
				->SetCullMode(CullMode::None);
		});

		// Animated Gerstner Water Surface pipeline
		const auto waterVert = m_resourceManager->LoadShader("Shaders/water/water.vert");
		const auto waterFrag = m_resourceManager->LoadShader("Shaders/water/water.frag");
		m_resourceManager->CreatePipeline("Water::Surface", [&](PipelineBlueprint* blueprint) {
			blueprint->SetVertexLayout(vertexLayout)
				->AddShader(waterVert)
				->AddShader(waterFrag)
				->SetPipelineLayout("Water")
				->SetColorFormat(TextureFormat::R16G16B16A16_SFLOAT)
				->SetHasDepthAttachment(true)
				->SetUseMsaa(false)
				->SetDepthTest(true)
				->SetDepthWrite(true)
				->SetBlendEnable(true)
				->SetAlphaToCoverage(false)
				->SetCullMode(CullMode::None);
		});

		// Forward Transparent pipeline
		const auto forwardVert = m_resourceManager->LoadShader("Shaders/main/main.vert");
		const auto forwardFrag = m_resourceManager->LoadShader("Shaders/main/main.frag");
		m_resourceManager->CreatePipeline("Forward::Transparent", [&](PipelineBlueprint* blueprint) {
			blueprint->SetVertexLayout(vertexLayout)
				->AddShader(forwardVert)
				->AddShader(forwardFrag)
				->SetPipelineLayout("Forward")
				->SetColorFormat(TextureFormat::R16G16B16A16_SFLOAT)
				->SetHasDepthAttachment(true)
				->SetUseMsaa(false)
				->SetDepthTest(true)
				->SetDepthWrite(false)
				->SetBlendEnable(true)
				->SetAlphaToCoverage(false);
		});

		// RT Shadow + AMD FidelityFX Shadow Denoiser pipelines
		if (m_rtSupported)
		{
			const auto rtShadowComp = m_resourceManager->LoadShader("Shaders/deferred/rt_shadows.comp");
			m_resourceManager->CreatePipeline("RTShadow::Trace", [&](PipelineBlueprint* blueprint) {
				blueprint->AddShader(rtShadowComp)->SetPipelineLayout("RTShadow");
			});

			const auto rtaoComp = m_resourceManager->LoadShader("Shaders/ao/rtao.comp");
			m_resourceManager->CreatePipeline("AO::RTAO", [&](PipelineBlueprint* blueprint) {
				blueprint->AddShader(rtaoComp)->SetPipelineLayout("RTAO");
			});
		}

		const auto ffxTileClassComp = m_resourceManager->LoadShader("Shaders/deferred/ffx_shadows_tile_classification.comp");
		m_resourceManager->CreatePipeline("RTShadow::FFXTileClassification", [&](PipelineBlueprint* blueprint) {
			blueprint->AddShader(ffxTileClassComp)->SetPipelineLayout("FFXShadowTileClass");
		});

		const auto ffxFilterComp = m_resourceManager->LoadShader("Shaders/deferred/ffx_shadows_filter.comp");
		m_resourceManager->CreatePipeline("RTShadow::FFXFilterSoftShadows", [&](PipelineBlueprint* blueprint) {
			blueprint->AddShader(ffxFilterComp)->SetPipelineLayout("FFXShadowFilter");
		});

		// Bend Studio Screen Space Shadows compute pipeline
		const auto bendSssComp = m_resourceManager->LoadShader("Shaders/deferred/bend_sss.comp");
		m_resourceManager->CreatePipeline("Bend::SSS", [&](PipelineBlueprint* blueprint) {
			blueprint->AddShader(bendSssComp)->SetPipelineLayout("BendSSS");
		});

		// AO pipelines
		const auto gtaoComp = m_resourceManager->LoadShader("Shaders/ao/gtao.comp");
		m_resourceManager->CreatePipeline("AO::GTAO", [&](PipelineBlueprint* blueprint) {
			blueprint->AddShader(gtaoComp)->SetPipelineLayout("GTAO");
		});

		const auto aoDenoiseComp = m_resourceManager->LoadShader("Shaders/ao/ao_denoise.comp");
		m_resourceManager->CreatePipeline("AO::Denoise", [&](PipelineBlueprint* blueprint) {
			blueprint->AddShader(aoDenoiseComp)->SetPipelineLayout("AODenoise");
		});

		// Hillaire Sky & Irradiance / IBL pipelines
		const auto skyTransComp = m_resourceManager->LoadShader("Shaders/atmosphere/transmittance_lut.comp");
		m_resourceManager->CreatePipeline("Sky::TransmittanceLUT", [&](PipelineBlueprint* blueprint) {
			blueprint->AddShader(skyTransComp)->SetPipelineLayout("SkyTransmittance");
		});

		const auto skyMsComp = m_resourceManager->LoadShader("Shaders/atmosphere/multiscattering_lut.comp");
		m_resourceManager->CreatePipeline("Sky::MultiScatteringLUT", [&](PipelineBlueprint* blueprint) {
			blueprint->AddShader(skyMsComp)->SetPipelineLayout("SkyMultiScattering");
		});

		const auto skyViewComp = m_resourceManager->LoadShader("Shaders/atmosphere/skyview_lut.comp");
		m_resourceManager->CreatePipeline("Sky::SkyViewLUT", [&](PipelineBlueprint* blueprint) {
			blueprint->AddShader(skyViewComp)->SetPipelineLayout("SkyView");
		});

		const auto skyIrrComp = m_resourceManager->LoadShader("Shaders/atmosphere/irradiance_convolve.comp");
		m_resourceManager->CreatePipeline("Sky::IrradianceConvolve", [&](PipelineBlueprint* blueprint) {
			blueprint->AddShader(skyIrrComp)->SetPipelineLayout("SkyIrradiance");
		});

		const auto skyPrefComp = m_resourceManager->LoadShader("Shaders/atmosphere/prefilter_env.comp");
		m_resourceManager->CreatePipeline("Sky::PrefilterEnv", [&](PipelineBlueprint* blueprint) {
			blueprint->AddShader(skyPrefComp)->SetPipelineLayout("SkyPrefilter");
		});

		const auto skyBrdfComp = m_resourceManager->LoadShader("Shaders/atmosphere/brdf_lut.comp");
		m_resourceManager->CreatePipeline("Sky::BrdfLUT", [&](PipelineBlueprint* blueprint) {
			blueprint->AddShader(skyBrdfComp)->SetPipelineLayout("SkyBrdf");
		});

		const auto skyRenderComp = m_resourceManager->LoadShader("Shaders/atmosphere/sky_render.comp");
		m_resourceManager->CreatePipeline("Sky::Render", [&](PipelineBlueprint* blueprint) {
			blueprint->AddShader(skyRenderComp)->SetPipelineLayout("SkyRender");
		});

		// Volumetric Raymarched Fog with Blue Noise (Shadertoy WsfBDf / Demofox)
		if (m_rtSupported)
		{
			const auto volFogRtComp = m_resourceManager->LoadShader("Shaders/atmosphere/volumetric_fog.comp");
			m_resourceManager->CreatePipeline("Atmosphere::VolumetricFogRT", [&](PipelineBlueprint* blueprint) {
				blueprint->AddShader(volFogRtComp)->SetPipelineLayout("VolumetricFogRT");
			});
		}
		const auto volFogNoRtComp = m_resourceManager->LoadShader("Shaders/atmosphere/volumetric_fog_no_rt.comp");
		m_resourceManager->CreatePipeline("Atmosphere::VolumetricFogNoRT", [&](PipelineBlueprint* blueprint) {
			blueprint->AddShader(volFogNoRtComp)->SetPipelineLayout("VolumetricFogNoRT");
		});

		// Deferred Lighting compute pipeline
		const auto deferredLightingComp = m_resourceManager->LoadShader("Shaders/deferred/deferred_lighting.comp");
		m_resourceManager->CreatePipeline("Deferred::Lighting", [&](PipelineBlueprint* blueprint) {
			blueprint->AddShader(deferredLightingComp)->SetPipelineLayout("DeferredLighting");
		});

		// Post-Process TAA & ToneMap pipelines
		const auto taaComp = m_resourceManager->LoadShader("Shaders/postprocess/taa.comp");
		m_resourceManager->CreatePipeline("PostProcess::TAA", [&](PipelineBlueprint* blueprint) {
			blueprint->AddShader(taaComp)->SetPipelineLayout("TAA");
		});

		const auto tonemapVert = m_resourceManager->LoadShader("Shaders/postprocess/tonemap.vert");
		const auto tonemapFrag = m_resourceManager->LoadShader("Shaders/postprocess/tonemap.frag");
		m_resourceManager->CreatePipeline("PostProcess::ToneMap", [&](PipelineBlueprint* blueprint) {
			blueprint->AddShader(tonemapVert)
				->AddShader(tonemapFrag)
				->SetPipelineLayout("ToneMap")
				->SetColorFormat(TextureFormat::Undefined)
				->SetHasDepthAttachment(false)
				->SetUseMsaa(false)
				->SetDepthTest(false)
				->SetDepthWrite(false)
				->SetBlendEnable(false)
				->SetAlphaToCoverage(false)
				->SetCullMode(CullMode::None);
		});
	}

	void Renderer::CreateBlueNoiseTexture()
	{
		// Generate a 64x64 toroidal Blue Noise texture via Ulichney's Void-and-Cluster algorithm
		constexpr uint32_t N = 64u;
		constexpr uint32_t Total = N * N;
		constexpr int32_t  KernelRadius = 4;
		constexpr float	   SigmaSq2 = 2.0f * 1.5f * 1.5f;

		std::array<float, (2 * KernelRadius + 1) * (2 * KernelRadius + 1)> kernel{};
		for (int32_t dy = -KernelRadius; dy <= KernelRadius; ++dy)
		{
			for (int32_t dx = -KernelRadius; dx <= KernelRadius; ++dx)
			{
				const float d2 = static_cast<float>(dx * dx + dy * dy);
				kernel[static_cast<size_t>((dy + KernelRadius) * (2 * KernelRadius + 1) + (dx + KernelRadius))] = std::exp(-d2 / SigmaSq2);
			}
		}

		std::vector<float>	 energy(Total, 0.0f);
		std::vector<uint8_t> occupied(Total, 0u);
		std::vector<uint32_t> rank(Total, 0u);

		auto splatEnergy = [&](const uint32_t px, const uint32_t py, const float sign) {
			for (int32_t dy = -KernelRadius; dy <= KernelRadius; ++dy)
			{
				const uint32_t ny = static_cast<uint32_t>((static_cast<int32_t>(py) + dy + static_cast<int32_t>(N)) & static_cast<int32_t>(N - 1u));
				const uint32_t rowOff = ny * N;
				const int32_t  kRow = (dy + KernelRadius) * (2 * KernelRadius + 1);
				for (int32_t dx = -KernelRadius; dx <= KernelRadius; ++dx)
				{
					const uint32_t nx = static_cast<uint32_t>((static_cast<int32_t>(px) + dx + static_cast<int32_t>(N)) & static_cast<int32_t>(N - 1u));
					energy[rowOff + nx] += sign * kernel[static_cast<size_t>(kRow + dx + KernelRadius)];
				}
			}
		};

		// Deterministic seed point
		uint32_t rng = 0x243F6A88u;
		auto nextRng = [&]() -> uint32_t {
			rng ^= rng << 13u;
			rng ^= rng >> 17u;
			rng ^= rng << 5u;
			return rng;
		};

		// Place first ~64 well-separated seed points using greedy void finding, then fill remaining 4032 ranks
		const uint32_t firstIdx = nextRng() % Total;
		occupied[firstIdx] = 1u;
		rank[firstIdx] = 0u;
		splatEnergy(firstIdx % N, firstIdx / N, 1.0f);

		for (uint32_t r = 1u; r < Total; ++r)
		{
			uint32_t bestIdx = 0u;
			float	 bestEnergy = 1e30f;
			for (uint32_t i = 0u; i < Total; ++i)
			{
				if (occupied[i])
					continue;
				if (energy[i] < bestEnergy)
				{
					bestEnergy = energy[i];
					bestIdx = i;
				}
			}
			occupied[bestIdx] = 1u;
			rank[bestIdx] = r;
			splatEnergy(bestIdx % N, bestIdx / N, 1.0f);
		}

		std::vector<uint8_t> pixels(Total * 4u);
		for (uint32_t i = 0u; i < Total; ++i)
		{
			const uint8_t v = static_cast<uint8_t>((rank[i] * 255u) / (Total - 1u));
			pixels[i * 4u + 0u] = v;
			pixels[i * 4u + 1u] = v;
			pixels[i * 4u + 2u] = v;
			pixels[i * 4u + 3u] = 255u;
		}

		TextureFromDataDescription desc{};
		desc.data = pixels.data();
		desc.dataSize = pixels.size();
		desc.width = N;
		desc.height = N;
		desc.channels = 4;
		desc.generateMipmaps = false;

		m_blueNoiseTexture = m_renderingDevice->CreateTextureFromData(desc);
	}

	void Renderer::CreateAtmosphereLuts()
	{
		auto makeLut = [&](const uint32_t width, const uint32_t height, const TextureFormat fmt) -> Handle {
			TextureCreateInfo info{};
			info.width = width;
			info.height = height;
			info.format = fmt;
			info.usage = TextureUsageFlags::Storage | TextureUsageFlags::Sampled;
			info.useMsaa = false;
			info.dedicated = false;
			info.mipLevels = 1;
			return m_renderingDevice->CreateTexture(info);
		};

		m_transmittanceLUT = makeLut(SkyAtmospherePass::kTransmittanceWidth, SkyAtmospherePass::kTransmittanceHeight, TextureFormat::R16G16B16A16_SFLOAT);
		m_multiScatteringLUT = makeLut(SkyAtmospherePass::kMultiScatteringSize, SkyAtmospherePass::kMultiScatteringSize, TextureFormat::R16G16B16A16_SFLOAT);
		m_skyViewLUT = makeLut(SkyAtmospherePass::kSkyViewWidth, SkyAtmospherePass::kSkyViewHeight, TextureFormat::R16G16B16A16_SFLOAT);
		m_irradianceLUT = makeLut(SkyAtmospherePass::kIrradianceWidth, SkyAtmospherePass::kIrradianceHeight, TextureFormat::R16G16B16A16_SFLOAT);
		m_prefilteredEnvLUT = makeLut(SkyAtmospherePass::kPrefilterWidth, SkyAtmospherePass::kPrefilterHeight, TextureFormat::R16G16B16A16_SFLOAT);
		m_brdfLUT = makeLut(SkyAtmospherePass::kBrdfLutSize, SkyAtmospherePass::kBrdfLutSize, TextureFormat::R16G16_SFLOAT);

		m_renderingDevice->UpdateDescriptorStorageImage(m_skyTransmittanceDescriptorSet, 0, m_transmittanceLUT);
		m_renderingDevice->UpdateDescriptorStorageImage(m_skyBrdfDescriptorSet, 0, m_brdfLUT);

		m_renderingDevice->UpdateDescriptorImage(m_skyMultiScatteringDescriptorSet, 0, m_transmittanceLUT, m_linearClampSampler);
		m_renderingDevice->UpdateDescriptorStorageImage(m_skyMultiScatteringDescriptorSet, 1, m_multiScatteringLUT);

		m_renderingDevice->UpdateDescriptorImage(m_skyViewDescriptorSet, 0, m_transmittanceLUT, m_linearClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_skyViewDescriptorSet, 1, m_multiScatteringLUT, m_linearClampSampler);
		m_renderingDevice->UpdateDescriptorStorageImage(m_skyViewDescriptorSet, 2, m_skyViewLUT);

		m_renderingDevice->UpdateDescriptorImage(m_skyIrradianceDescriptorSet, 0, m_skyViewLUT, m_skyViewSampler);
		m_renderingDevice->UpdateDescriptorStorageImage(m_skyIrradianceDescriptorSet, 1, m_irradianceLUT);

		m_renderingDevice->UpdateDescriptorImage(m_skyPrefilterDescriptorSet, 0, m_skyViewLUT, m_skyViewSampler);
		m_renderingDevice->UpdateDescriptorStorageImage(m_skyPrefilterDescriptorSet, 1, m_prefilteredEnvLUT);

		m_renderingDevice->UpdateDescriptorImage(m_forwardIblDescriptorSet, 0, m_transmittanceLUT, m_linearClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_forwardIblDescriptorSet, 1, m_irradianceLUT, m_skyViewSampler);
		m_renderingDevice->UpdateDescriptorImage(m_forwardIblDescriptorSet, 2, m_prefilteredEnvLUT, m_skyViewSampler);
		m_renderingDevice->UpdateDescriptorImage(m_forwardIblDescriptorSet, 3, m_brdfLUT, m_linearClampSampler);

		m_atmosphereStaticLutsDirty = true;
		m_brdfLutDirty = true;
	}

	void Renderer::EnsureRenderTargets()
	{
		const uint64_t currentGen = m_renderingDevice->GetSwapChainGeneration();
		const auto [displayWidth, displayHeight] = m_renderingDevice->GetSwapChainExtent();
		if (displayWidth == 0 || displayHeight == 0)
			return;

		auto& settings = RenderSettings::Get();
		const bool fsrSelected = (settings.upscalerMode == EUpscalerMode::FSR && m_renderingDevice->IsFfxUpscalerAvailable());

		uint32_t renderWidth = displayWidth;
		uint32_t renderHeight = displayHeight;
		if (fsrSelected)
		{
			m_renderingDevice->EnsureFfxUpscalerContext(
				displayWidth,
				displayHeight,
				displayWidth,
				displayHeight,
				settings.fsrSelectedVersionIndex
			);
			const auto [queriedW, queriedH] = m_renderingDevice->QueryFfxRenderResolution(
				displayWidth,
				displayHeight,
				static_cast<uint32_t>(settings.fsrQualityMode)
			);
			if (queriedW > 0 && queriedH > 0)
			{
				renderWidth = std::min(queriedW, displayWidth);
				renderHeight = std::min(queriedH, displayHeight);
			}
		}

		settings.fsrDisplayWidth = displayWidth;
		settings.fsrDisplayHeight = displayHeight;
		settings.fsrRenderWidth = renderWidth;
		settings.fsrRenderHeight = renderHeight;

		if (settings.enableFrameGeneration && m_renderingDevice->IsFfxFrameGenAvailable())
		{
			m_renderingDevice->EnsureFfxFrameGenContext(
				displayWidth,
				displayHeight,
				displayWidth,
				displayHeight,
				settings.fgSelectedVersionIndex
			);
		}

		if (currentGen == m_lastSwapChainGeneration &&
			m_depthTarget.IsValid() &&
			m_renderWidth == renderWidth &&
			m_renderHeight == renderHeight &&
			m_displayWidth == displayWidth &&
			m_displayHeight == displayHeight &&
			m_lastUpscalerMode == settings.upscalerMode &&
			m_lastFsrQualityMode == settings.fsrQualityMode &&
			m_lastFsrVersionIndex == settings.fsrSelectedVersionIndex &&
			m_lastFgVersionIndex == settings.fgSelectedVersionIndex)
		{
			return;
		}

		if (m_depthTarget.IsValid())
			m_renderingDevice->WaitIdle();

		DestroyRenderTargets();

		m_renderWidth = renderWidth;
		m_renderHeight = renderHeight;
		m_displayWidth = displayWidth;
		m_displayHeight = displayHeight;
		m_lastUpscalerMode = settings.upscalerMode;
		m_lastFsrQualityMode = settings.fsrQualityMode;
		m_lastFsrVersionIndex = settings.fsrSelectedVersionIndex;
		m_lastFgVersionIndex = settings.fgSelectedVersionIndex;

		auto makeTarget = [&](const uint32_t w, const uint32_t h, const TextureFormat fmt, const TextureUsageFlags usage) -> Handle {
			TextureCreateInfo info{};
			info.width = w;
			info.height = h;
			info.format = fmt;
			info.usage = usage;
			info.useMsaa = false;
			info.dedicated = true;
			info.mipLevels = 1;
			return m_renderingDevice->CreateTexture(info);
		};

		// G-Buffer targets + previous-frame geometry history (at render resolution)
		m_gbufferAlbedo = makeTarget(renderWidth, renderHeight, TextureFormat::R8G8B8A8_UNORM, TextureUsageFlags::ColorAttachment | TextureUsageFlags::Sampled);
		m_gbufferNormal = makeTarget(renderWidth, renderHeight, TextureFormat::R16G16_SFLOAT, TextureUsageFlags::ColorAttachment | TextureUsageFlags::Sampled | TextureUsageFlags::TransferSrc);
		m_gbufferMaterial = makeTarget(renderWidth, renderHeight, TextureFormat::R8G8B8A8_UNORM, TextureUsageFlags::ColorAttachment | TextureUsageFlags::Sampled);
		m_gbufferMotion = makeTarget(renderWidth, renderHeight, TextureFormat::R16G16_SFLOAT, TextureUsageFlags::ColorAttachment | TextureUsageFlags::Storage | TextureUsageFlags::Sampled);
		m_depthTarget = m_renderingDevice->CreateDepthAttachment(renderWidth, renderHeight, false);
		m_prevDepthTarget = makeTarget(renderWidth, renderHeight, TextureFormat::D32_SFLOAT, TextureUsageFlags::TransferDst | TextureUsageFlags::Sampled);
		m_prevNormalTarget = makeTarget(renderWidth, renderHeight, TextureFormat::R16G16_SFLOAT, TextureUsageFlags::TransferDst | TextureUsageFlags::Sampled);

		// RT Shadow & AMD FidelityFX Shadow Denoiser targets + 8x4 tile SSBOs (at render resolution)
		const auto storageSampled = TextureUsageFlags::Storage | TextureUsageFlags::Sampled | TextureUsageFlags::TransferSrc | TextureUsageFlags::TransferDst;
		m_rtShadowRaw = makeTarget(renderWidth, renderHeight, TextureFormat::R16G16_SFLOAT, storageSampled);
		m_rtShadowPing = makeTarget(renderWidth, renderHeight, TextureFormat::R16G16_SFLOAT, storageSampled);
		m_rtShadowPong = makeTarget(renderWidth, renderHeight, TextureFormat::R16G16_SFLOAT, storageSampled);
		m_rtShadowDenoised = makeTarget(renderWidth, renderHeight, TextureFormat::R16G16_SFLOAT, storageSampled);
		m_rtShadowMoments[0] = makeTarget(renderWidth, renderHeight, TextureFormat::R16G16B16A16_SFLOAT, storageSampled);
		m_rtShadowMoments[1] = makeTarget(renderWidth, renderHeight, TextureFormat::R16G16B16A16_SFLOAT, storageSampled);

		const uint32_t tilesX = (renderWidth + 7u) / 8u;
		const uint32_t tilesY = (renderHeight + 3u) / 4u;
		m_rtShadowTileBufferBytes = std::max<size_t>(1, static_cast<size_t>(tilesX) * tilesY) * sizeof(uint32_t);
		{
			BufferDescription tileBufDesc{};
			tileBufDesc.size = m_rtShadowTileBufferBytes;
			tileBufDesc.bufferUsage = BufferUsageFlags::Storage;
			tileBufDesc.memoryUsage = BufferMemoryUsage::GpuOnly;
			m_rtShadowTileMaskBuffer = m_renderingDevice->CreateBuffer(tileBufDesc);
			m_rtShadowTileMetaBuffer = m_renderingDevice->CreateBuffer(tileBufDesc);
		}

		// Bend Studio Screen Space Shadow target (at render resolution)
		m_screenSpaceShadow = makeTarget(renderWidth, renderHeight, TextureFormat::R16G16_SFLOAT, TextureUsageFlags::Storage | TextureUsageFlags::Sampled);

		// Ambient Occlusion targets (at render resolution)
		m_aoRaw = makeTarget(renderWidth, renderHeight, TextureFormat::R16G16_SFLOAT, storageSampled);
		m_aoPing = makeTarget(renderWidth, renderHeight, TextureFormat::R16G16_SFLOAT, storageSampled);
		m_aoPong = makeTarget(renderWidth, renderHeight, TextureFormat::R16G16_SFLOAT, storageSampled);
		m_aoDenoised = makeTarget(renderWidth, renderHeight, TextureFormat::R16G16_SFLOAT, storageSampled);
		m_aoHistory = makeTarget(renderWidth, renderHeight, TextureFormat::R16G16_SFLOAT, storageSampled);

		// HDR lighting buffer & Water refraction snapshots (at render resolution)
		m_hdrColor = makeTarget(renderWidth, renderHeight, TextureFormat::R16G16B16A16_SFLOAT, TextureUsageFlags::ColorAttachment | TextureUsageFlags::Storage | TextureUsageFlags::Sampled | TextureUsageFlags::TransferSrc);
		m_waterSceneColorCopy = makeTarget(renderWidth, renderHeight, TextureFormat::R16G16B16A16_SFLOAT, TextureUsageFlags::TransferDst | TextureUsageFlags::Sampled);
		m_waterDepthCopy = makeTarget(renderWidth, renderHeight, TextureFormat::D32_SFLOAT, TextureUsageFlags::TransferDst | TextureUsageFlags::Sampled);

		// Resolved / Upscaled HDR output targets & AMD Frame Generation interpolated target (at display resolution)
		m_taaHistory[0] = makeTarget(displayWidth, displayHeight, TextureFormat::R16G16B16A16_SFLOAT, TextureUsageFlags::Storage | TextureUsageFlags::Sampled);
		m_taaHistory[1] = makeTarget(displayWidth, displayHeight, TextureFormat::R16G16B16A16_SFLOAT, TextureUsageFlags::Storage | TextureUsageFlags::Sampled);
		m_fgInterpolatedColor = makeTarget(displayWidth, displayHeight, TextureFormat::R16G16B16A16_SFLOAT, TextureUsageFlags::Storage | TextureUsageFlags::Sampled);

		m_lastSwapChainGeneration = currentGen;
		m_historyValid = false;

		UpdateRenderTargetDescriptorSets();
	}

	void Renderer::UpdateRenderTargetDescriptorSets()
	{
		if (m_rtSupported)
		{
			for (size_t i = 0; i < m_rtShadowDescriptorSets.size(); ++i)
			{
				const Handle shadowSet = m_rtShadowDescriptorSets[i];
				m_renderingDevice->UpdateDescriptorImage(shadowSet, 1, m_depthTarget, m_nearestClampSampler);
				m_renderingDevice->UpdateDescriptorImage(shadowSet, 2, m_gbufferNormal, m_nearestClampSampler);
				m_renderingDevice->UpdateDescriptorStorageImage(shadowSet, 3, m_rtShadowRaw);
				m_renderingDevice->UpdateDescriptorBuffer(shadowSet, 4, m_rtShadowTileMaskBuffer, m_rtShadowTileBufferBytes, DescriptorType::StorageBuffer);

				const Handle rtaoSet = m_rtaoDescriptorSets[i];
				m_renderingDevice->UpdateDescriptorImage(rtaoSet, 1, m_depthTarget, m_nearestClampSampler);
				m_renderingDevice->UpdateDescriptorImage(rtaoSet, 2, m_gbufferNormal, m_nearestClampSampler);
				m_renderingDevice->UpdateDescriptorStorageImage(rtaoSet, 3, m_aoRaw);
				m_renderingDevice->UpdateDescriptorImage(rtaoSet, 4, m_blueNoiseTexture, m_nearestRepeatSampler);

				const Handle fogRtSet = m_volumetricFogRtDescriptorSets[i];
				m_renderingDevice->UpdateDescriptorImage(fogRtSet, 1, m_depthTarget, m_nearestClampSampler);
				m_renderingDevice->UpdateDescriptorImage(fogRtSet, 2, m_blueNoiseTexture, m_nearestRepeatSampler);
				m_renderingDevice->UpdateDescriptorImage(fogRtSet, 3, m_transmittanceLUT, m_linearClampSampler);
				m_renderingDevice->UpdateDescriptorImage(fogRtSet, 4, m_irradianceLUT, m_skyViewSampler);
				m_renderingDevice->UpdateDescriptorStorageImage(fogRtSet, 5, m_hdrColor);

				if (m_tlasHandles[i].IsValid())
				{
					m_renderingDevice->UpdateDescriptorAccelerationStructure(shadowSet, 0, m_tlasHandles[i]);
					m_renderingDevice->UpdateDescriptorAccelerationStructure(rtaoSet, 0, m_tlasHandles[i]);
					m_renderingDevice->UpdateDescriptorAccelerationStructure(fogRtSet, 0, m_tlasHandles[i]);
					m_tlasBoundToSets[i] = true;
				}
				else
				{
					m_tlasBoundToSets[i] = false;
				}
			}
		}

		// AMD FFX Shadow Denoiser - Stage 1 (ping-pong moments by writeIdx)
		for (uint32_t writeIdx = 0; writeIdx < 2; ++writeIdx)
		{
			const uint32_t readIdx = 1u - writeIdx;
			const Handle   set = m_ffxTileClassDescriptorSets[writeIdx];
			m_renderingDevice->UpdateDescriptorBuffer(set, 0, m_rtShadowTileMaskBuffer, m_rtShadowTileBufferBytes, DescriptorType::StorageBuffer);
			m_renderingDevice->UpdateDescriptorBuffer(set, 1, m_rtShadowTileMetaBuffer, m_rtShadowTileBufferBytes, DescriptorType::StorageBuffer);
			m_renderingDevice->UpdateDescriptorImage(set, 2, m_rtShadowRaw, m_nearestClampSampler);
			m_renderingDevice->UpdateDescriptorImage(set, 3, m_rtShadowMoments[readIdx], m_linearClampSampler);
			m_renderingDevice->UpdateDescriptorImage(set, 4, m_depthTarget, m_nearestClampSampler);
			m_renderingDevice->UpdateDescriptorImage(set, 5, m_prevDepthTarget, m_nearestClampSampler);
			m_renderingDevice->UpdateDescriptorImage(set, 6, m_gbufferNormal, m_nearestClampSampler);
			m_renderingDevice->UpdateDescriptorImage(set, 7, m_prevNormalTarget, m_nearestClampSampler);
			m_renderingDevice->UpdateDescriptorImage(set, 8, m_gbufferMotion, m_nearestClampSampler);
			m_renderingDevice->UpdateDescriptorStorageImage(set, 9, m_rtShadowMoments[writeIdx]);
			m_renderingDevice->UpdateDescriptorStorageImage(set, 10, m_rtShadowPing);
		}

		// AMD FFX Shadow Denoiser - Stage 2 (3 A-Trous passes: ping->pong, pong->ping, ping->denoised)
		const std::array<Handle, 3> filterSrc = {m_rtShadowPing, m_rtShadowPong, m_rtShadowPing};
		const std::array<Handle, 3> filterDst = {m_rtShadowPong, m_rtShadowPing, m_rtShadowDenoised};
		for (uint32_t passIdx = 0; passIdx < 3; ++passIdx)
		{
			const Handle set = m_ffxFilterDescriptorSets[passIdx];
			m_renderingDevice->UpdateDescriptorBuffer(set, 0, m_rtShadowTileMetaBuffer, m_rtShadowTileBufferBytes, DescriptorType::StorageBuffer);
			m_renderingDevice->UpdateDescriptorImage(set, 1, filterSrc[passIdx], m_nearestClampSampler);
			m_renderingDevice->UpdateDescriptorImage(set, 2, m_depthTarget, m_nearestClampSampler);
			m_renderingDevice->UpdateDescriptorImage(set, 3, m_gbufferNormal, m_nearestClampSampler);
			m_renderingDevice->UpdateDescriptorStorageImage(set, 4, filterDst[passIdx]);
		}

		// Bend Studio Screen Space Shadow set (Nearest + ClampToBorder white = FarDepthValue 1.0)
		m_renderingDevice->UpdateDescriptorImage(m_bendSssDescriptorSet, 0, m_depthTarget, m_pointBorderWhiteSampler);
		m_renderingDevice->UpdateDescriptorStorageImage(m_bendSssDescriptorSet, 1, m_screenSpaceShadow);

		// GTAO & AO Denoise sets
		m_renderingDevice->UpdateDescriptorImage(m_gtaoDescriptorSet, 0, m_depthTarget, m_nearestClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_gtaoDescriptorSet, 1, m_gbufferNormal, m_nearestClampSampler);
		m_renderingDevice->UpdateDescriptorStorageImage(m_gtaoDescriptorSet, 2, m_aoRaw);
		m_renderingDevice->UpdateDescriptorImage(m_gtaoDescriptorSet, 3, m_blueNoiseTexture, m_nearestRepeatSampler);

		m_renderingDevice->UpdateDescriptorImage(m_aoDenoiseDescriptorSet, 0, m_aoRaw, m_nearestClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_aoDenoiseDescriptorSet, 1, m_aoHistory, m_linearClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_aoDenoiseDescriptorSet, 2, m_depthTarget, m_nearestClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_aoDenoiseDescriptorSet, 3, m_prevDepthTarget, m_nearestClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_aoDenoiseDescriptorSet, 4, m_gbufferNormal, m_nearestClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_aoDenoiseDescriptorSet, 5, m_prevNormalTarget, m_nearestClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_aoDenoiseDescriptorSet, 6, m_gbufferMotion, m_nearestClampSampler);
		m_renderingDevice->UpdateDescriptorStorageImage(m_aoDenoiseDescriptorSet, 7, m_aoPing);

		// AO Stage 2: 3 A-Trous passes (ping->pong, pong->ping, ping->denoised)
		const std::array<Handle, 3> aoFilterSrc = {m_aoPing, m_aoPong, m_aoPing};
		const std::array<Handle, 3> aoFilterDst = {m_aoPong, m_aoPing, m_aoDenoised};
		for (uint32_t passIdx = 0; passIdx < 3; ++passIdx)
		{
			const Handle set = m_aoFilterDescriptorSets[passIdx];
			m_renderingDevice->UpdateDescriptorImage(set, 0, aoFilterSrc[passIdx], m_nearestClampSampler);
			m_renderingDevice->UpdateDescriptorImage(set, 1, m_aoHistory, m_linearClampSampler);
			m_renderingDevice->UpdateDescriptorImage(set, 2, m_depthTarget, m_nearestClampSampler);
			m_renderingDevice->UpdateDescriptorImage(set, 3, m_prevDepthTarget, m_nearestClampSampler);
			m_renderingDevice->UpdateDescriptorImage(set, 4, m_gbufferNormal, m_nearestClampSampler);
			m_renderingDevice->UpdateDescriptorImage(set, 5, m_prevNormalTarget, m_nearestClampSampler);
			m_renderingDevice->UpdateDescriptorImage(set, 6, m_gbufferMotion, m_nearestClampSampler);
			m_renderingDevice->UpdateDescriptorStorageImage(set, 7, aoFilterDst[passIdx]);
		}

		// Sky Render set
		m_renderingDevice->UpdateDescriptorImage(m_skyRenderDescriptorSet, 0, m_depthTarget, m_nearestClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_skyRenderDescriptorSet, 1, m_skyViewLUT, m_skyViewSampler);
		m_renderingDevice->UpdateDescriptorImage(m_skyRenderDescriptorSet, 2, m_transmittanceLUT, m_linearClampSampler);
		m_renderingDevice->UpdateDescriptorStorageImage(m_skyRenderDescriptorSet, 3, m_hdrColor);
		m_renderingDevice->UpdateDescriptorStorageImage(m_skyRenderDescriptorSet, 4, m_gbufferMotion);

		// Deferred Lighting set (full PBR + Irradiance IBL + AO + FFX Shadows)
		m_renderingDevice->UpdateDescriptorImage(m_deferredLightingDescriptorSet, 0, m_gbufferAlbedo, m_nearestClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_deferredLightingDescriptorSet, 1, m_gbufferNormal, m_nearestClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_deferredLightingDescriptorSet, 2, m_gbufferMaterial, m_nearestClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_deferredLightingDescriptorSet, 3, m_gbufferMotion, m_nearestClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_deferredLightingDescriptorSet, 4, m_depthTarget, m_nearestClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_deferredLightingDescriptorSet, 5, m_rtShadowDenoised, m_nearestClampSampler);
		m_renderingDevice->UpdateDescriptorStorageImage(m_deferredLightingDescriptorSet, 6, m_hdrColor);
		m_renderingDevice->UpdateDescriptorImage(m_deferredLightingDescriptorSet, 7, m_transmittanceLUT, m_linearClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_deferredLightingDescriptorSet, 8, m_irradianceLUT, m_skyViewSampler);
		m_renderingDevice->UpdateDescriptorImage(m_deferredLightingDescriptorSet, 9, m_prefilteredEnvLUT, m_skyViewSampler);
		m_renderingDevice->UpdateDescriptorImage(m_deferredLightingDescriptorSet, 10, m_brdfLUT, m_linearClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_deferredLightingDescriptorSet, 11, m_aoDenoised, m_nearestClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_deferredLightingDescriptorSet, 12, m_screenSpaceShadow, m_nearestClampSampler);

		// Water Surface set (underwater depth & refracted HDR snapshots + Hillaire IBL)
		m_renderingDevice->UpdateDescriptorImage(m_waterDescriptorSet, 0, m_waterDepthCopy, m_nearestClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_waterDescriptorSet, 1, m_waterSceneColorCopy, m_linearClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_waterDescriptorSet, 2, m_transmittanceLUT, m_linearClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_waterDescriptorSet, 3, m_irradianceLUT, m_skyViewSampler);
		m_renderingDevice->UpdateDescriptorImage(m_waterDescriptorSet, 4, m_prefilteredEnvLUT, m_skyViewSampler);
		m_renderingDevice->UpdateDescriptorImage(m_waterDescriptorSet, 5, m_brdfLUT, m_linearClampSampler);

		// Volumetric Fog fallback set (without HW RT)
		m_renderingDevice->UpdateDescriptorImage(m_volumetricFogNoRtDescriptorSet, 0, m_depthTarget, m_nearestClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_volumetricFogNoRtDescriptorSet, 1, m_blueNoiseTexture, m_nearestRepeatSampler);
		m_renderingDevice->UpdateDescriptorImage(m_volumetricFogNoRtDescriptorSet, 2, m_transmittanceLUT, m_linearClampSampler);
		m_renderingDevice->UpdateDescriptorImage(m_volumetricFogNoRtDescriptorSet, 3, m_irradianceLUT, m_skyViewSampler);
		m_renderingDevice->UpdateDescriptorStorageImage(m_volumetricFogNoRtDescriptorSet, 4, m_hdrColor);

		// TAA ping-pong & ToneMap sets (use linearClampSampler for currentHdrTex so non-FSR fallback can upsample smoothly)
		for (uint32_t writeIdx = 0; writeIdx < 2; ++writeIdx)
		{
			const uint32_t readIdx = 1u - writeIdx;
			m_renderingDevice->UpdateDescriptorImage(m_taaDescriptorSets[writeIdx], 0, m_hdrColor, m_linearClampSampler);
			m_renderingDevice->UpdateDescriptorImage(m_taaDescriptorSets[writeIdx], 1, m_taaHistory[readIdx], m_linearClampSampler);
			m_renderingDevice->UpdateDescriptorImage(m_taaDescriptorSets[writeIdx], 2, m_gbufferMotion, m_nearestClampSampler);
			m_renderingDevice->UpdateDescriptorImage(m_taaDescriptorSets[writeIdx], 3, m_depthTarget, m_nearestClampSampler);
			m_renderingDevice->UpdateDescriptorStorageImage(m_taaDescriptorSets[writeIdx], 4, m_taaHistory[writeIdx]);

			m_renderingDevice->UpdateDescriptorImage(m_toneMapDescriptorSets[writeIdx], 0, m_taaHistory[writeIdx], m_linearClampSampler);
		}
		m_renderingDevice->UpdateDescriptorImage(m_fgToneMapDescriptorSet, 0, m_fgInterpolatedColor, m_linearClampSampler);
	}

	void Renderer::DestroyRenderTargets()
	{
		auto destroyTex = [&](Handle& handle) {
			if (handle.IsValid())
			{
				m_renderingDevice->DestroyTexture(handle);
				handle = {};
			}
		};
		auto destroyBuf = [&](Handle& handle) {
			if (handle.IsValid())
			{
				m_renderingDevice->DestroyBuffer(handle);
				handle = {};
			}
		};

		destroyTex(m_gbufferAlbedo);
		destroyTex(m_gbufferNormal);
		destroyTex(m_gbufferMaterial);
		destroyTex(m_gbufferMotion);
		destroyTex(m_depthTarget);
		destroyTex(m_prevDepthTarget);
		destroyTex(m_prevNormalTarget);

		destroyTex(m_rtShadowRaw);
		destroyTex(m_rtShadowPing);
		destroyTex(m_rtShadowPong);
		destroyTex(m_rtShadowDenoised);
		destroyTex(m_rtShadowMoments[0]);
		destroyTex(m_rtShadowMoments[1]);
		destroyBuf(m_rtShadowTileMaskBuffer);
		destroyBuf(m_rtShadowTileMetaBuffer);

		destroyTex(m_screenSpaceShadow);

		destroyTex(m_aoRaw);
		destroyTex(m_aoPing);
		destroyTex(m_aoPong);
		destroyTex(m_aoDenoised);
		destroyTex(m_aoHistory);

		destroyTex(m_hdrColor);
		destroyTex(m_waterSceneColorCopy);
		destroyTex(m_waterDepthCopy);
		destroyTex(m_taaHistory[0]);
		destroyTex(m_taaHistory[1]);
		destroyTex(m_fgInterpolatedColor);
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
		const uint32_t renderWidth = m_renderWidth;
		const uint32_t renderHeight = m_renderHeight;
		const uint32_t displayWidth = m_displayWidth;
		const uint32_t displayHeight = m_displayHeight;

		const bool tlasBuilt = BuildAccelerationStructures(scene, frameIndex);
		BuildDrawStreams(scene, frameIndex, renderWidth, renderHeight, tlasBuilt);

		const auto&	   settings = RenderSettings::Get();
		const uint32_t readIdx = m_temporalReadIndex;
		const uint32_t writeIdx = 1u - readIdx;
		const Camera*  camera = scene->GetCamera();

		if (settings.atmosphereDensityScale != m_lastAtmosphereDensityScale)
		{
			m_atmosphereStaticLutsDirty = true;
			m_skyIblLutsDirty = true;
			m_lastAtmosphereDensityScale = settings.atmosphereDensityScale;
		}
		if (settings.atmosphereAltitudeKm != m_lastAtmosphereAltitudeKm ||
			settings.atmosphereMieAnisotropy != m_lastAtmosphereMieAnisotropy ||
			settings.atmosphereMultiScatteringFactor != m_lastAtmosphereMultiScatter ||
			Vector3::Distance2(m_currentSunDirection, m_lastSunDirection) > 1e-6f)
		{
			m_skyIblLutsDirty = true;
			m_lastAtmosphereAltitudeKm = settings.atmosphereAltitudeKm;
			m_lastAtmosphereMieAnisotropy = settings.atmosphereMieAnisotropy;
			m_lastAtmosphereMultiScatter = settings.atmosphereMultiScatteringFactor;
			m_lastSunDirection = m_currentSunDirection;
		}

		RenderPassContext ctx{};
		ctx.renderingDevice = m_renderingDevice;
		ctx.resourceManager = m_resourceManager;
		ctx.editor = m_editor;
		ctx.scene = scene;
		ctx.frameIndex = frameIndex;
		ctx.swapChainImageIndex = imageIndex;
		ctx.viewportWidth = renderWidth;
		ctx.viewportHeight = renderHeight;
		ctx.displayWidth = displayWidth;
		ctx.displayHeight = displayHeight;
		ctx.rtSupported = m_rtSupported;
		ctx.rtShadowsActive = m_rtSupported && settings.enableRTShadows && tlasBuilt;
		ctx.bendSssActive = settings.enableBendSSS;
		ctx.tlasValid = tlasBuilt;
		ctx.activeAoMode = (settings.aoMode == EAOMode::RTAO && !m_rtSupported) ? EAOMode::GTAO : settings.aoMode;
		ctx.atmosphereStaticLutsDirty = m_atmosphereStaticLutsDirty;
		ctx.skyIblLutsDirty = m_skyIblLutsDirty;
		ctx.brdfLutDirty = m_brdfLutDirty;
		ctx.temporalReadIndex = readIdx;
		ctx.temporalWriteIndex = writeIdx;
		ctx.fsrUpscaleActive = (settings.upscalerMode == EUpscalerMode::FSR && m_renderingDevice->IsFfxUpscalerAvailable());
		ctx.fsrResetHistory = !m_historyValid;
		ctx.fgActive = (settings.enableFrameGeneration && m_renderingDevice->IsFfxFrameGenAvailable());
		ctx.jitterPixelsX = m_currentJitterPixelsX;
		ctx.jitterPixelsY = m_currentJitterPixelsY;
		ctx.frameTimeDeltaMs = m_currentFrameDeltaMs;
		ctx.cameraNear = camera->GetNearPlane();
		ctx.cameraFar = camera->GetFarPlane();
		ctx.cameraFovVerticalRadians = camera->GetFov() * (3.14159265359f / 180.0f);
		ctx.cameraPreExposure = camera->GetExposure();
		ctx.cameraPosition = camera->GetPosition();
		ctx.cameraUp = camera->GetUp();
		ctx.cameraRight = camera->GetRight();
		ctx.cameraForward = camera->GetForward();
		ctx.unjitteredViewProj = m_prevUnjitteredViewProj;
		ctx.sunDirection = m_currentSunDirection;

		ctx.gbufferAlbedo = m_gbufferAlbedo;
		ctx.gbufferNormal = m_gbufferNormal;
		ctx.gbufferMaterial = m_gbufferMaterial;
		ctx.gbufferMotion = m_gbufferMotion;
		ctx.depthTarget = m_depthTarget;
		ctx.prevDepthTarget = m_prevDepthTarget;
		ctx.prevNormalTarget = m_prevNormalTarget;

		ctx.rtShadowRaw = m_rtShadowRaw;
		ctx.rtShadowPing = m_rtShadowPing;
		ctx.rtShadowPong = m_rtShadowPong;
		ctx.rtShadowDenoised = m_rtShadowDenoised;
		ctx.rtShadowMoments = m_rtShadowMoments;
		ctx.screenSpaceShadow = m_screenSpaceShadow;

		ctx.aoRaw = m_aoRaw;
		ctx.aoPing = m_aoPing;
		ctx.aoPong = m_aoPong;
		ctx.aoDenoised = m_aoDenoised;
		ctx.aoHistory = m_aoHistory;

		ctx.transmittanceLUT = m_transmittanceLUT;
		ctx.multiScatteringLUT = m_multiScatteringLUT;
		ctx.skyViewLUT = m_skyViewLUT;
		ctx.irradianceLUT = m_irradianceLUT;
		ctx.prefilteredEnvLUT = m_prefilteredEnvLUT;
		ctx.brdfLUT = m_brdfLUT;

		ctx.hdrColor = m_hdrColor;
		ctx.waterSceneColorCopy = m_waterSceneColorCopy;
		ctx.waterDepthCopy = m_waterDepthCopy;
		ctx.taaHistory = m_taaHistory;
		ctx.fgInterpolatedColor = m_fgInterpolatedColor;

		ctx.globalDescriptorSet = m_globalDescriptorSets[frameIndex];
		ctx.pointLightDescriptorSet = m_pointLightDescriptorSets[frameIndex];
		ctx.rtShadowDescriptorSet = m_rtSupported ? m_rtShadowDescriptorSets[frameIndex] : Handle{};
		ctx.ffxTileClassDescriptorSets = m_ffxTileClassDescriptorSets;
		ctx.ffxFilterDescriptorSets = m_ffxFilterDescriptorSets;
		ctx.bendSssDescriptorSet = m_bendSssDescriptorSet;
		ctx.rtaoDescriptorSet = m_rtSupported ? m_rtaoDescriptorSets[frameIndex] : Handle{};
		ctx.gtaoDescriptorSet = m_gtaoDescriptorSet;
		ctx.aoDenoiseDescriptorSet = m_aoDenoiseDescriptorSet;
		ctx.aoFilterDescriptorSets = m_aoFilterDescriptorSets;
		ctx.skyTransmittanceDescriptorSet = m_skyTransmittanceDescriptorSet;
		ctx.skyMultiScatteringDescriptorSet = m_skyMultiScatteringDescriptorSet;
		ctx.skyViewDescriptorSet = m_skyViewDescriptorSet;
		ctx.skyIrradianceDescriptorSet = m_skyIrradianceDescriptorSet;
		ctx.skyPrefilterDescriptorSet = m_skyPrefilterDescriptorSet;
		ctx.skyBrdfDescriptorSet = m_skyBrdfDescriptorSet;
		ctx.skyRenderDescriptorSet = m_skyRenderDescriptorSet;
		ctx.deferredLightingDescriptorSet = m_deferredLightingDescriptorSet;
		ctx.waterDescriptorSet = m_waterDescriptorSet;
		ctx.volumetricFogRtDescriptorSet = m_rtSupported ? m_volumetricFogRtDescriptorSets[frameIndex] : Handle{};
		ctx.volumetricFogNoRtDescriptorSet = m_volumetricFogNoRtDescriptorSet;
		ctx.taaDescriptorSets = m_taaDescriptorSets;
		ctx.toneMapDescriptorSets = m_toneMapDescriptorSets;
		ctx.fgToneMapDescriptorSet = m_fgToneMapDescriptorSet;

		ctx.opaqueStream = m_opaqueDrawStream.get();
		ctx.waterStream = m_waterDrawStream.get();
		ctx.transparentStream = m_transparentDrawStream.get();

		for (const auto& pass : m_passes)
			pass->Execute(ctx);

		// Snapshot current frame's depth & normals for next frame's FFX Shadow Denoiser & AO disocclusion checks
		m_renderingDevice->CopyTexture(m_depthTarget, m_prevDepthTarget);
		m_renderingDevice->CopyTexture(m_gbufferNormal, m_prevNormalTarget);

		m_renderingDevice->EndFrame(imageIndex);

		m_atmosphereStaticLutsDirty = false;
		m_skyIblLutsDirty = false;
		m_brdfLutDirty = false;
		m_temporalReadIndex = writeIdx;
		m_historyValid = true;
		++m_frameCounter;
	}

	bool Renderer::BuildAccelerationStructures(Scene* scene, const uint32_t frameIndex)
	{
		const auto& settings = RenderSettings::Get();
		const bool	needTlas = m_rtSupported && (settings.enableRTShadows || settings.aoMode == EAOMode::RTAO || ((settings.enableVolumetricFog || settings.enableVolumetricShadows) && settings.fogUseRTShadows));
		if (!needTlas)
			return false;

		m_tlasInstancesScratch.clear();

		const auto& registry = scene->GetRegistry();
		const auto	meshView = scene->GetView<TransformComponent, MeshComponent>();
		for (auto [entity, transform, meshComp] : meshView.each())
		{
			if (meshComp.meshId == InvalidResourceId ||
				registry.all_of<WaterChunkComponent>(entity) ||
				registry.all_of<GrassChunkComponent>(entity))
				continue;

			const MeshResourceCache* meshData = m_resourceManager->GetMeshData(meshComp.meshId);
			if (!meshData || meshData->vertexCount == 0 || meshData->indexCount < 3)
				continue;

			if (meshComp.materialId != InvalidResourceId)
			{
				const MaterialResourceCache* material = m_resourceManager->GetMaterialData(meshComp.materialId);
				if (material && material->alphaMode == EAlphaMode::Blend)
					continue;
			}

			Handle blasHandle{};
			if (const auto it = m_blasCache.find(meshComp.meshId); it != m_blasCache.end())
			{
				blasHandle = it->second;
			}
			else
			{
				blasHandle = m_renderingDevice->CreateBlas(
					meshData->vertexBuffer,
					meshData->vertexCount,
					sizeof(VertexData),
					meshData->indexBuffer,
					meshData->indexCount
				);
				m_blasCache[meshComp.meshId] = blasHandle;
			}

			if (!blasHandle.IsValid())
				continue;

			const glm::mat4& m = transform.worldMatrix.Value();
			TlasInstanceDesc inst{};
			inst.blasHandle = blasHandle;
			inst.instanceCustomIndex = static_cast<uint32_t>(m_tlasInstancesScratch.size());
			inst.mask = 0xFF;
			inst.transform[0] = m[0][0];
			inst.transform[1] = m[1][0];
			inst.transform[2] = m[2][0];
			inst.transform[3] = m[3][0];
			inst.transform[4] = m[0][1];
			inst.transform[5] = m[1][1];
			inst.transform[6] = m[2][1];
			inst.transform[7] = m[3][1];
			inst.transform[8] = m[0][2];
			inst.transform[9] = m[1][2];
			inst.transform[10] = m[2][2];
			inst.transform[11] = m[3][2];
			m_tlasInstancesScratch.push_back(inst);
		}

		if (m_tlasInstancesScratch.empty())
			return false;

		const Handle prevTlas = m_tlasHandles[frameIndex];
		m_renderingDevice->BuildOrUpdateTlas(
			m_tlasHandles[frameIndex],
			m_tlasScratchBuffers[frameIndex],
			m_tlasInstanceBuffers[frameIndex],
			m_tlasInstancesScratch
		);

		if (m_tlasHandles[frameIndex].IsValid() && (m_tlasHandles[frameIndex] != prevTlas || !m_tlasBoundToSets[frameIndex]))
		{
			m_renderingDevice->UpdateDescriptorAccelerationStructure(m_rtShadowDescriptorSets[frameIndex], 0, m_tlasHandles[frameIndex]);
			m_renderingDevice->UpdateDescriptorAccelerationStructure(m_rtaoDescriptorSets[frameIndex], 0, m_tlasHandles[frameIndex]);
			m_renderingDevice->UpdateDescriptorAccelerationStructure(m_volumetricFogRtDescriptorSets[frameIndex], 0, m_tlasHandles[frameIndex]);
			m_tlasBoundToSets[frameIndex] = true;
		}

		return m_tlasHandles[frameIndex].IsValid();
	}

	void Renderer::BuildDrawStreams(Scene* scene, const uint32_t frameIndex, const uint32_t viewportWidth, const uint32_t viewportHeight, const bool tlasValid)
	{
		const float	  aspectRatio = m_renderingDevice->GetAspectRatio();
		const Camera* camera = scene->GetCamera();
		auto&		  settings = RenderSettings::Get();

		const bool taaOrFsrEnabled = (settings.upscalerMode != EUpscalerMode::Off);
		settings.enableTAA = taaOrFsrEnabled;

		if (taaOrFsrEnabled != m_prevTaaEnabled)
		{
			m_historyValid = false;
			m_prevTaaEnabled = taaOrFsrEnabled;
		}

		Vector3 dirLightDirection = Vector3(0.0f, -1.0f, 0.0f);
		Vector3 dirLightColor = Vector3::One;
		float	sunIlluminanceLux = 0.0f;
		if (const auto dirLightView = scene->GetView<DirectionalLightComponent>(); dirLightView.begin() != dirLightView.end())
		{
			const auto& dirLight = dirLightView.get<DirectionalLightComponent>(*dirLightView.begin());
			dirLightDirection = dirLight.direction;
			dirLightColor = dirLight.color;
			sunIlluminanceLux = dirLight.illuminanceLux;
		}

		const uint32_t pointLightCount = UploadPointLights(scene, frameIndex);

		const Matrix4& viewMatrix = camera->GetViewMatrix();
		const Matrix4  unjitteredProj = camera->GetProjectionMatrix(aspectRatio);
		const Matrix4  unjitteredViewProj = unjitteredProj * viewMatrix;

		m_currentJitterPixelsX = 0.0f;
		m_currentJitterPixelsY = 0.0f;

		Matrix4 projMatrix = unjitteredProj;
		if (taaOrFsrEnabled && settings.debugView == EDebugViewMode::Final && viewportWidth > 0 && viewportHeight > 0)
		{
			const bool useFsrJitter = (settings.upscalerMode == EUpscalerMode::FSR && m_renderingDevice->IsFfxUpscalerAvailable());
			if (useFsrJitter)
			{
				const uint32_t displayW = m_displayWidth > 0 ? m_displayWidth : viewportWidth;
				const int32_t  phaseCount = m_renderingDevice->QueryFfxJitterPhaseCount(viewportWidth, displayW);
				const int32_t  jitterIdx = static_cast<int32_t>(m_frameCounter % static_cast<uint64_t>(std::max(phaseCount, 1)));
				const auto [jx, jy] = m_renderingDevice->QueryFfxJitterOffset(jitterIdx, phaseCount);
				m_currentJitterPixelsX = jx;
				m_currentJitterPixelsY = jy;
			}
			else
			{
				const uint32_t sampleIdx = static_cast<uint32_t>((m_frameCounter % 16u) + 1u);
				m_currentJitterPixelsX = Halton(sampleIdx, 2) - 0.5f;
				m_currentJitterPixelsY = Halton(sampleIdx, 3) - 0.5f;
			}

			// Note: Matrix4::Perspective flips m[1][1] negative for Vulkan Y-down clip space,
			// so NDC jitter is (+2*jx/w, +2*jy/h)
			const float ndcJitterX = (2.0f * m_currentJitterPixelsX) / static_cast<float>(viewportWidth);
			const float ndcJitterY = (2.0f * m_currentJitterPixelsY) / static_cast<float>(viewportHeight);
			projMatrix = Matrix4::Identity.Translate(ndcJitterX, ndcJitterY, 0.0f) * unjitteredProj;
		}

		const Matrix4 viewProj = projMatrix * viewMatrix;
		const Matrix4 invViewProj = viewProj.Inverted();
		const Matrix4 prevViewProj = m_historyValid ? m_prevUnjitteredViewProj : unjitteredViewProj;
		const Frustum frustum = Frustum::FromViewProjection(unjitteredViewProj);

		uint32_t renderFlags = 0;
		if (m_rtSupported && settings.enableRTShadows && tlasValid)
			renderFlags |= GlobalRenderFlags::RTShadowsEnabled;
		if (settings.enableShadowDenoiser)
			renderFlags |= GlobalRenderFlags::ShadowDenoiserEnabled;
		if (settings.upscalerMode == EUpscalerMode::TAA)
			renderFlags |= GlobalRenderFlags::TAAEnabled;
		if (m_historyValid)
			renderFlags |= GlobalRenderFlags::HistoryValid;
		if (settings.aoMode != EAOMode::Off)
			renderFlags |= GlobalRenderFlags::AOEnabled;
		if (settings.enableBendSSS)
			renderFlags |= GlobalRenderFlags::BendSSSEnabled;
		if (settings.enableVolumetricClouds)
			renderFlags |= GlobalRenderFlags::VolumetricCloudsEnabled;

		m_currentSunDirection = (dirLightDirection * -1.0f).Normalized();
		if (m_currentSunDirection.SquaredMagnitude() < 1e-6f)
			m_currentSunDirection = Vector3(0.0f, 1.0f, 0.0f);

		const auto globalUniform = GetGlobalUniformBuffer(frameIndex);
		globalUniform->projection = projMatrix;
		globalUniform->view = viewMatrix;
		globalUniform->viewProj = viewProj;
		globalUniform->invViewProj = invViewProj;
		globalUniform->unjitteredViewProj = unjitteredViewProj;
		globalUniform->prevUnjitteredViewProj = prevViewProj;
		globalUniform->lightDirection = dirLightDirection;
		globalUniform->sunAngularRadius = settings.sunAngularRadius;
		globalUniform->lightColor = dirLightColor;
		globalUniform->sunIlluminanceLux = sunIlluminanceLux;
		globalUniform->cameraPosition = camera->GetPosition();
		globalUniform->cameraExposure = camera->GetExposure();
		globalUniform->pointLightCount = pointLightCount;
		globalUniform->frameIndex = static_cast<uint32_t>(m_frameCounter);
		globalUniform->screenWidth = viewportWidth;
		globalUniform->screenHeight = viewportHeight;
		globalUniform->debugView = static_cast<uint32_t>(settings.debugView);
		globalUniform->toneMapMode = static_cast<uint32_t>(settings.toneMapMode);
		globalUniform->flags = renderFlags;
		globalUniform->taaBlendFactor = settings.taaBlendFactor;
		globalUniform->skyLuminanceNits = settings.skyLuminanceNits;
		globalUniform->atmosphereAltitudeKm = settings.atmosphereAltitudeKm;
		globalUniform->atmosphereMieAnisotropy = settings.atmosphereMieAnisotropy;
		globalUniform->atmosphereMultiScatteringFactor = settings.atmosphereMultiScatteringFactor;
		globalUniform->atmosphereDensityScale = settings.atmosphereDensityScale;
		const float elapsedTime = std::chrono::duration<float>(std::chrono::steady_clock::now() - m_startTime).count();
		if (m_frameCounter > 0 && elapsedTime > m_lastFrameElapsedSeconds)
			m_currentFrameDeltaMs = std::clamp((elapsedTime - m_lastFrameElapsedSeconds) * 1000.0f, 1.0f, 250.0f);
		else
			m_currentFrameDeltaMs = 16.67f;
		m_lastFrameElapsedSeconds = elapsedTime;

		globalUniform->aoRadius = settings.aoRadius;
		globalUniform->aoPower = settings.aoPower;
		globalUniform->irradianceIntensity = settings.irradianceIntensity;
		globalUniform->time = elapsedTime;
		globalUniform->waterLevel = settings.waterLevel;
		globalUniform->waterWaveScale = settings.waterWaveScale;
		globalUniform->waterFoamIntensity = settings.waterFoamIntensity;
		globalUniform->grassWindStrength = settings.grassWindStrength;
		globalUniform->grassWindSpeed = settings.grassWindSpeed;
		globalUniform->grassFadeDistance = settings.grassFadeDistance;
		globalUniform->_padGrass = 0.0f;
		globalUniform->cloudCoverage = settings.cloudCoverage;
		globalUniform->cloudDensity = settings.cloudDensity;
		globalUniform->cloudSpeed = settings.cloudSpeed;
		globalUniform->cloudShadowStrength = settings.cloudShadowStrength;

		m_prevUnjitteredViewProj = unjitteredViewProj;

		Metrics::SetCameraCoordinates(camera->GetPosition().GetX(), camera->GetPosition().GetY(), camera->GetPosition().GetZ());

		const auto& registry = scene->GetRegistry();
		const auto	meshView = scene->GetView<TransformComponent, MeshComponent>();
		const auto	estimatedCount = static_cast<uint32_t>(meshView.size_hint());
		m_opaqueDrawStream->Reset(estimatedCount);
		m_waterDrawStream->Reset(64);
		m_transparentDrawStream->Reset(estimatedCount / 4);

		const PipelineResourceCache* waterPipeline = m_resourceManager->GetPipelineData("Water::Surface");

		const Handle  globalSetHandle = m_globalDescriptorSets[frameIndex];
		const Handle  pointLightSetHandle = m_pointLightDescriptorSets[frameIndex];
		const Vector3 cameraPos = camera->GetPosition();

		for (auto [entity, transform, meshComp] : meshView.each())
		{
			if (meshComp.meshId == InvalidResourceId)
				continue;

			if (!settings.enableProceduralGrass && registry.all_of<GrassChunkComponent>(entity))
				continue;

			const MeshResourceCache* meshData = m_resourceManager->GetMeshData(meshComp.meshId);
			if (!meshData)
				continue;

			if (!frustum.IntersectsLocalAABB(meshData->boundsMin, meshData->boundsMax, transform.worldMatrix))
			{
				Metrics::AddCulledDrawCall();
				continue;
			}

			if (registry.all_of<WaterChunkComponent>(entity))
			{
				DrawPacket waterPacket{};
				waterPacket.pipeline = waterPipeline->pipeline;
				waterPacket.pipelineLayoutName = waterPipeline->layoutName;
				waterPacket.vertexBuffer = meshData->vertexBuffer;
				waterPacket.indexBuffer = meshData->indexBuffer;
				waterPacket.indexCount = meshData->indexCount;
				waterPacket.descriptorSets[0] = globalSetHandle;
				waterPacket.descriptorSets[1] = m_waterDescriptorSet;
				waterPacket.descriptorSetCount = 2;
				waterPacket.SetPushConstant(DynamicUniformData{transform.worldMatrix, transform.prevWorldMatrix}, ShaderStageFlags::Vertex);
				waterPacket.distanceToCamera = Vector3::Distance2(cameraPos, transform.GetWorldPosition());
				waterPacket.sortBackToFront = false;
				m_waterDrawStream->Draw(waterPacket);
				Metrics::AddDrawCall(meshData->vertexCount);
				continue;
			}

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
			if (isTransparent)
			{
				packet.descriptorSets[2] = pointLightSetHandle;
				packet.descriptorSets[3] = m_forwardIblDescriptorSet;
				packet.descriptorSetCount = 4;
			}
			else
			{
				packet.descriptorSetCount = 2;
			}
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
