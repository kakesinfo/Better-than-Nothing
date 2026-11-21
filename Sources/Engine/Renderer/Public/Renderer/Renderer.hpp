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

#pragma once

#include "RenderPass.hpp"
#include "RenderingDevice/Handle.hpp"
#include "RenderingDevice/RenderingDevice.hpp"
#include <Common/ResourceId.hpp>
#include <Data/Graphics/PointLightData.hpp>
#include <Math/Matrix4.hpp>
#include <array>
#include <chrono>
#include <memory>
#include <unordered_map>
#include <vector>

namespace BetterThanNothing
{
	class ResourceManager;
	class RenderingDevice;
	class Scene;
	class Editor;
	class DrawStream;
	struct GlobalUniformData;

	class Renderer final
	{
	public:
		explicit Renderer(RenderingDevice* renderingDevice, ResourceManager* resourceManager, Editor* editor);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		static std::unique_ptr<Renderer> Create(RenderingDevice* renderingDevice, ResourceManager* resourceManager, Editor* editor);

		void AddPass(std::unique_ptr<IRenderPass> pass);
		void RenderFrame(Scene* scene);

	private:
		void CreateLayoutsAndPipelines();
		void CreateAtmosphereLuts();
		void CreateBlueNoiseTexture();
		void CreateGlobalUniformBuffers();
		void CreateGlobalUniformDescriptorSets();
		auto GetGlobalUniformBuffer(uint32_t frameIndex) const -> GlobalUniformData*;

		void CreatePointLightBuffers();
		void CreatePointLightDescriptorSets();

		void EnsureRenderTargets();
		void DestroyRenderTargets();
		void UpdateRenderTargetDescriptorSets();

		void	 BuildDrawStreams(Scene* scene, uint32_t frameIndex, uint32_t viewportWidth, uint32_t viewportHeight, bool tlasValid);
		bool	 BuildAccelerationStructures(Scene* scene, uint32_t frameIndex);
		uint32_t UploadPointLights(const Scene* scene, uint32_t frameIndex) const;

		RenderingDevice*			m_renderingDevice = nullptr;
		ResourceManager*			m_resourceManager = nullptr;
		Editor*						m_editor = nullptr;
		std::unique_ptr<DrawStream> m_opaqueDrawStream;
		std::unique_ptr<DrawStream> m_waterDrawStream;
		std::unique_ptr<DrawStream> m_transparentDrawStream;

		std::vector<std::unique_ptr<IRenderPass>> m_passes;

		// Samplers
		Handle m_nearestClampSampler{};
		Handle m_nearestRepeatSampler{};
		Handle m_pointBorderWhiteSampler{};
		Handle m_linearClampSampler{};
		Handle m_skyViewSampler{};

		// G-Buffer targets & previous-frame geometry history
		Handle m_gbufferAlbedo{};
		Handle m_gbufferNormal{};
		Handle m_gbufferMaterial{};
		Handle m_gbufferMotion{};
		Handle m_depthTarget{};
		Handle m_prevDepthTarget{};
		Handle m_prevNormalTarget{};

		// RT Shadow & AMD FidelityFX Shadow Denoiser targets + tile buffers
		Handle				  m_rtShadowRaw{};
		Handle				  m_rtShadowPing{};
		Handle				  m_rtShadowPong{};
		Handle				  m_rtShadowDenoised{};
		std::array<Handle, 2> m_rtShadowMoments{};
		Handle				  m_rtShadowTileMaskBuffer{};
		Handle				  m_rtShadowTileMetaBuffer{};
		size_t				  m_rtShadowTileBufferBytes = 0;

		// Bend Studio Screen Space Shadow target
		Handle m_screenSpaceShadow{};

		// Ambient Occlusion targets
		Handle m_aoRaw{};
		Handle m_aoPing{};
		Handle m_aoPong{};
		Handle m_aoDenoised{};
		Handle m_aoHistory{};

		// Hillaire Sky & Irradiance / IBL LUTs (resolution-independent)
		Handle m_transmittanceLUT{};
		Handle m_multiScatteringLUT{};
		Handle m_skyViewLUT{};
		Handle m_irradianceLUT{};
		Handle m_prefilteredEnvLUT{};
		Handle m_brdfLUT{};
		Handle	m_blueNoiseTexture{};
		bool	m_atmosphereStaticLutsDirty = true;
		bool	m_skyIblLutsDirty = true;
		bool	m_brdfLutDirty = true;
		float	m_lastAtmosphereDensityScale = 1.0f;
		float	m_lastAtmosphereAltitudeKm = -1.0f;
		float	m_lastAtmosphereMieAnisotropy = -1.0f;
		float	m_lastAtmosphereMultiScatter = -1.0f;
		Vector3 m_lastSunDirection = Vector3::Zero;

		// HDR, Water snapshots & TAA / FSR Upscaled targets
		Handle				  m_hdrColor{};
		Handle				  m_waterSceneColorCopy{};
		Handle				  m_waterDepthCopy{};
		std::array<Handle, 2> m_taaHistory{};
		uint64_t			  m_lastSwapChainGeneration = 0;
		uint32_t			  m_renderWidth = 0;
		uint32_t			  m_renderHeight = 0;
		uint32_t			  m_displayWidth = 0;
		uint32_t			  m_displayHeight = 0;
		EUpscalerMode		  m_lastUpscalerMode = EUpscalerMode::FSR;
		EFsrQualityMode		  m_lastFsrQualityMode = EFsrQualityMode::NativeAA;
		uint32_t			  m_lastFsrVersionIndex = 0;
		float				  m_currentJitterPixelsX = 0.0f;
		float				  m_currentJitterPixelsY = 0.0f;
		float				  m_lastFrameElapsedSeconds = 0.0f;
		float				  m_currentFrameDeltaMs = 16.67f;

		// Pass descriptor sets
		std::vector<Handle>	  m_rtShadowDescriptorSets;
		std::vector<Handle>	  m_rtaoDescriptorSets;
		std::vector<Handle>	  m_volumetricFogRtDescriptorSets;
		std::vector<bool>	  m_tlasBoundToSets;
		std::array<Handle, 2> m_ffxTileClassDescriptorSets{};
		std::array<Handle, 3> m_ffxFilterDescriptorSets{};
		Handle				  m_bendSssDescriptorSet{};
		Handle				  m_gtaoDescriptorSet{};
		Handle				  m_aoDenoiseDescriptorSet{};
		std::array<Handle, 3> m_aoFilterDescriptorSets{};
		Handle				  m_skyTransmittanceDescriptorSet{};
		Handle				  m_skyMultiScatteringDescriptorSet{};
		Handle				  m_skyViewDescriptorSet{};
		Handle				  m_skyIrradianceDescriptorSet{};
		Handle				  m_skyPrefilterDescriptorSet{};
		Handle				  m_skyBrdfDescriptorSet{};
		Handle				  m_skyRenderDescriptorSet{};
		Handle				  m_deferredLightingDescriptorSet{};
		Handle				  m_waterDescriptorSet{};
		Handle				  m_volumetricFogNoRtDescriptorSet{};
		Handle				  m_forwardIblDescriptorSet{};
		std::array<Handle, 2> m_taaDescriptorSets{};
		std::array<Handle, 2> m_toneMapDescriptorSets{};

		// Ray Tracing acceleration structures
		bool								   m_rtSupported = false;
		std::unordered_map<ResourceId, Handle> m_blasCache;
		std::vector<Handle>					   m_tlasHandles;
		std::vector<Handle>					   m_tlasScratchBuffers;
		std::vector<Handle>					   m_tlasInstanceBuffers;
		std::vector<TlasInstanceDesc>		   m_tlasInstancesScratch;

		// Per-frame uniforms & lights
		std::vector<Handle>					m_globalUniformBuffer;
		std::vector<Handle>					m_globalDescriptorSets;
		std::vector<Handle>					m_pointLightBuffer;
		std::vector<Handle>					m_pointLightDescriptorSets;
		mutable std::vector<PointLightData> m_pointLightsScratch;
		mutable bool						m_warnedLightLimit = false;

		// Temporal state
		std::chrono::steady_clock::time_point m_startTime = std::chrono::steady_clock::now();
		Matrix4								  m_prevUnjitteredViewProj = Matrix4::Identity;
		Vector3								  m_currentSunDirection = Vector3(0.0f, 1.0f, 0.0f);
		uint64_t							  m_frameCounter = 0;
		bool	 m_historyValid = false;
		bool	 m_prevTaaEnabled = true;
		uint32_t m_temporalReadIndex = 0;
	};
}
