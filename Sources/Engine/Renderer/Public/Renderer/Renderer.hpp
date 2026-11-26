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
#include <Data/Graphics/PointLightData.hpp>
#include <Math/Matrix4.hpp>
#include <RenderingDevice/SamplerDescription.hpp>
#include <RenderingDevice/Types.hpp>
#include <memory>
#include <vector>

namespace BetterThanNothing
{
	class ResourceManager;
	class RenderingDevice;
	class Scene;
	class Editor;
	class DrawStream;
	struct GlobalUniformData;

	struct AtmosphereLutSpec
	{
		uint32_t	  width = 0;
		uint32_t	  height = 0;
		TextureFormat format = TextureFormat::Undefined;
	};

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

		[[nodiscard]] static float				Halton(uint32_t index, uint32_t base);
		[[nodiscard]] static uint32_t			ComputeRTShadowTileCount(uint32_t width, uint32_t height);
		[[nodiscard]] static SamplerDescription MakePointBorderWhiteSamplerDesc();
		[[nodiscard]] static SamplerDescription MakeSkyViewSamplerDesc();

		[[nodiscard]] static AtmosphereLutSpec GetTransmittanceLutSpec();
		[[nodiscard]] static AtmosphereLutSpec GetMultiScatteringLutSpec();
		[[nodiscard]] static AtmosphereLutSpec GetSkyViewLutSpec();
		[[nodiscard]] static AtmosphereLutSpec GetIrradianceLutSpec();
		[[nodiscard]] static AtmosphereLutSpec GetPrefilteredEnvLutSpec();
		[[nodiscard]] static AtmosphereLutSpec GetBrdfLutSpec();
		[[nodiscard]] static bool			   ShouldIncludeInAccelerationStructure(bool isGrassChunk) noexcept { return !isGrassChunk; }

		void AddPass(std::unique_ptr<IRenderPass> pass);
		void RenderFrame(Scene* scene);

	private:
		void CreateGlobalUniformBuffers();
		void CreateGlobalUniformDescriptorSets();
		auto GetGlobalUniformBuffer(uint32_t frameIndex) const -> GlobalUniformData*;

		void CreatePointLightBuffers();
		void CreatePointLightDescriptorSets();

		void EnsureRenderTargets();
		void DestroyRenderTargets();
		void CreateAtmosphereLuts();
		void DestroyAtmosphereLuts();

		void	 BuildDrawStreams(Scene* scene, uint32_t frameIndex);
		uint32_t UploadPointLights(const Scene* scene, uint32_t frameIndex) const;

		RenderingDevice*			m_renderingDevice = nullptr;
		ResourceManager*			m_resourceManager = nullptr;
		Editor*						m_editor = nullptr;
		std::unique_ptr<DrawStream> m_opaqueDrawStream;
		std::unique_ptr<DrawStream> m_transparentDrawStream;

		std::vector<std::unique_ptr<IRenderPass>> m_passes;

		Handle	 m_colorTarget{};
		Handle	 m_depthTarget{};
		Handle	 m_gbufferAlbedo{};
		Handle	 m_gbufferNormal{};
		Handle	 m_gbufferMaterial{};
		Handle	 m_gbufferMotion{};
		Handle	 m_prevDepthTarget{};
		Handle	 m_prevNormalTarget{};
		Handle	 m_hdrColor{};
		Handle	 m_taaHistory[2]{};
		Handle	 m_rtShadowRaw{};
		Handle	 m_rtShadowPing{};
		Handle	 m_rtShadowPong{};
		Handle	 m_rtShadowDenoised{};
		Handle	 m_rtShadowMoments[2]{};
		Handle	 m_sssTarget{};
		Handle	 m_aoRaw{};
		Handle	 m_aoDenoised{};
		Handle	 m_aoHistory{};
		Handle	 m_rtShadowTileMaskBuffer{};
		Handle	 m_rtShadowTileMetaBuffer{};
		Handle	 m_transmittanceLUT{};
		Handle	 m_multiScatteringLUT{};
		Handle	 m_skyViewLUT{};
		Handle	 m_irradianceLUT{};
		Handle	 m_prefilteredEnvLUT{};
		Handle	 m_brdfLUT{};
		uint32_t m_temporalReadIndex = 0;
		uint64_t m_lastSwapChainGeneration = 0;
		Matrix4	 m_prevUnjitteredViewProj = Matrix4::Identity;
		uint32_t m_frameIndex = 0;

		std::vector<Handle>					m_globalUniformBuffer;
		std::vector<Handle>					m_globalDescriptorSets;
		std::vector<Handle>					m_pointLightBuffer;
		std::vector<Handle>					m_pointLightDescriptorSets;
		std::vector<Handle>					m_deferredDescriptorSets;
		std::vector<Handle>					m_taaDescriptorSets[2];
		std::vector<Handle>					m_tonemapDescriptorSets[2];
		std::vector<Handle>					m_rtShadowTraceDescriptorSets;
		std::vector<Handle>					m_rtShadowClassifyDescriptorSets[2];
		std::vector<Handle>					m_rtShadowFilterDescriptorSets[3];
		std::vector<Handle>					m_sssDescriptorSets;
		std::vector<Handle>					m_rtaoDescriptorSets;
		std::vector<Handle>					m_gtaoDescriptorSets;
		std::vector<Handle>					m_aoDenoiseDescriptorSets;
		Handle								m_skyBrdfDescriptorSet{};
		Handle								m_skyTransmittanceDescriptorSet{};
		Handle								m_skyMultiScatteringDescriptorSet{};
		Handle								m_skyViewDescriptorSet{};
		Handle								m_skyIrradianceDescriptorSet{};
		Handle								m_skyPrefilterDescriptorSet{};
		std::vector<Handle>					m_skyRenderDescriptorSets;
		Vector3								m_lightDirection{0.0f, -1.0f, 0.0f};
		Handle								m_computeNearestSampler{};
		Handle								m_linearClampSampler{};
		Handle								m_pointBorderWhiteSampler{};
		Handle								m_skyViewSampler{};
		mutable std::vector<PointLightData> m_pointLightsScratch;
		mutable bool						m_warnedLightLimit = false;
	};
}
