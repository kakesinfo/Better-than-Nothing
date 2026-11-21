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

#include <Common/RenderSettings.hpp>
#include <Math/Matrix4.hpp>
#include <Math/Vector3.hpp>
#include <RenderingDevice/DrawStream.hpp>
#include <RenderingDevice/Handle.hpp>
#include <array>
#include <cstdint>
#include <string_view>

namespace BetterThanNothing
{
	class RenderingDevice;
	class ResourceManager;
	class Scene;
	class Editor;

	struct RenderPassContext
	{
		RenderingDevice* renderingDevice = nullptr;
		ResourceManager* resourceManager = nullptr;
		Editor*			 editor = nullptr;
		Scene*			 scene = nullptr;

		uint32_t frameIndex = 0;
		uint32_t swapChainImageIndex = 0;
		uint32_t viewportWidth = 0;
		uint32_t viewportHeight = 0;

		bool	 rtSupported = false;
		bool	 rtShadowsActive = false;
		bool	 bendSssActive = false;
		bool	 tlasValid = false;
		EAOMode	 activeAoMode = EAOMode::GTAO;
		bool	 atmosphereStaticLutsDirty = true;
		bool	 brdfLutDirty = true;
		uint32_t temporalReadIndex = 0;
		uint32_t temporalWriteIndex = 1;

		Matrix4 unjitteredViewProj = Matrix4::Identity;
		Vector3 sunDirection = Vector3(0.0f, 1.0f, 0.0f);

		// G-Buffer targets & previous-frame geometry history
		Handle gbufferAlbedo{};
		Handle gbufferNormal{};
		Handle gbufferMaterial{};
		Handle gbufferMotion{};
		Handle depthTarget{};
		Handle prevDepthTarget{};
		Handle prevNormalTarget{};

		// RT Shadow & AMD FFX Shadow Denoiser targets
		Handle				  rtShadowRaw{};
		Handle				  rtShadowPing{};
		Handle				  rtShadowPong{};
		Handle				  rtShadowDenoised{};
		std::array<Handle, 2> rtShadowMoments{};

		// Bend Studio Screen Space Shadow target
		Handle screenSpaceShadow{};

		// Ambient Occlusion targets
		Handle aoRaw{};
		Handle aoDenoised{};
		Handle aoHistory{};

		// Hillaire Sky & Irradiance / IBL LUTs
		Handle transmittanceLUT{};
		Handle multiScatteringLUT{};
		Handle skyViewLUT{};
		Handle irradianceLUT{};
		Handle prefilteredEnvLUT{};
		Handle brdfLUT{};

		// HDR, Water snapshots & TAA targets
		Handle				  hdrColor{};
		Handle				  waterSceneColorCopy{};
		Handle				  waterDepthCopy{};
		std::array<Handle, 2> taaHistory{};

		// Descriptor sets
		Handle				  globalDescriptorSet{};
		Handle				  pointLightDescriptorSet{};
		Handle				  rtShadowDescriptorSet{};
		std::array<Handle, 2> ffxTileClassDescriptorSets{};
		std::array<Handle, 3> ffxFilterDescriptorSets{}; // pass 0: ping->pong, pass 1: pong->ping, pass 2: ping->denoised
		Handle				  bendSssDescriptorSet{};
		Handle				  rtaoDescriptorSet{};
		Handle				  gtaoDescriptorSet{};
		Handle				  aoDenoiseDescriptorSet{};
		Handle				  skyTransmittanceDescriptorSet{};
		Handle				  skyMultiScatteringDescriptorSet{};
		Handle				  skyViewDescriptorSet{};
		Handle				  skyIrradianceDescriptorSet{};
		Handle				  skyPrefilterDescriptorSet{};
		Handle				  skyBrdfDescriptorSet{};
		Handle				  skyRenderDescriptorSet{};
		Handle				  deferredLightingDescriptorSet{};
		Handle				  waterDescriptorSet{};
		Handle				  volumetricFogRtDescriptorSet{};
		Handle				  volumetricFogNoRtDescriptorSet{};
		std::array<Handle, 2> taaDescriptorSets{};
		std::array<Handle, 2> toneMapDescriptorSets{};

		DrawStream* opaqueStream = nullptr;
		DrawStream* waterStream = nullptr;
		DrawStream* transparentStream = nullptr;
	};

	class IRenderPass
	{
	public:
		virtual ~IRenderPass() = default;

		[[nodiscard]] virtual std::string_view GetName() const = 0;
		virtual void						   Execute(RenderPassContext& ctx) = 0;
	};
}
