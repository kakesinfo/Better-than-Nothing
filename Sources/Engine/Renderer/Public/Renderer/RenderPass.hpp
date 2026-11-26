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

#include <Math/Matrix4.hpp>
#include <Math/Vector3.hpp>
#include <RenderingDevice/DrawStream.hpp>
#include <RenderingDevice/Handle.hpp>
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
		bool	 useMsaa = false;

		Handle colorTarget{};
		Handle depthTarget{};
		Handle gbufferAlbedo{};
		Handle gbufferNormal{};
		Handle gbufferMaterial{};
		Handle gbufferMotion{};
		Handle prevDepthTarget{};
		Handle prevNormalTarget{};
		Handle hdrColor{};
		Handle taaHistoryRead{};
		Handle taaResolved{};
		Handle rtShadowRaw{};
		Handle rtShadowPing{};
		Handle rtShadowPong{};
		Handle rtShadowDenoised{};
		Handle rtShadowMomentsRead{};
		Handle rtShadowMomentsWrite{};
		Handle sssTarget{};
		Handle aoRaw{};
		Handle aoDenoised{};
		Handle aoHistory{};
		Handle rtShadowTileMaskBuffer{};
		Handle rtShadowTileMetaBuffer{};
		Handle topLevelAS{};
		Handle transmittanceLUT{};
		Handle multiScatteringLUT{};
		Handle skyViewLUT{};
		Handle irradianceLUT{};
		Handle prefilteredEnvLUT{};
		Handle brdfLUT{};
		Handle globalDescriptorSet{};
		Handle deferredLightingDescriptorSet{};
		Handle taaDescriptorSet{};
		Handle tonemapDescriptorSet{};
		Handle rtShadowTraceDescriptorSet{};
		Handle rtShadowClassifyDescriptorSet{};
		Handle rtShadowFilterDescriptorSets[3]{};
		Handle sssDescriptorSet{};
		Handle rtaoDescriptorSet{};
		Handle gtaoDescriptorSet{};
		Handle aoDenoiseDescriptorSet{};

		Matrix4 unjitteredViewProj = Matrix4::Identity;
		Vector3 lightDirection{0.0f, -1.0f, 0.0f};

		DrawStream* opaqueStream = nullptr;
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
