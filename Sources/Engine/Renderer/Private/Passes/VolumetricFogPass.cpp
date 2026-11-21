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

#include "VolumetricFogPass.hpp"

#include <Common/RenderSettings.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>
#include <algorithm>
#include <array>

namespace BetterThanNothing
{
	namespace
	{
		constexpr uint32_t kFogFlagBlueNoise = 1u << 0;
		constexpr uint32_t kFogFlagRTShadows = 1u << 1;
		constexpr uint32_t kFogFlagRestrictToBox = 1u << 2;
		constexpr uint32_t kFogFlagVolumetricShadows = 1u << 3;
	}

	void VolumetricFogPass::Execute(RenderPassContext& ctx)
	{
		const auto& settings = RenderSettings::Get();
		if (!settings.enableVolumetricFog && !settings.enableVolumetricShadows && settings.debugView != EDebugViewMode::VolumetricFog)
			return;
		if (ctx.viewportWidth == 0 || ctx.viewportHeight == 0)
			return;

		RenderingDevice* renderingDevice = ctx.renderingDevice;
		ResourceManager* resourceManager = ctx.resourceManager;

		const bool	 useRtPipeline = ctx.rtSupported && ctx.tlasValid && ctx.volumetricFogRtDescriptorSet.IsValid();
		const Handle fogSet = useRtPipeline ? ctx.volumetricFogRtDescriptorSet : ctx.volumetricFogNoRtDescriptorSet;
		if (!fogSet.IsValid())
			return;

		renderingDevice->TransitionTexture(ctx.depthTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.transmittanceLUT, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.irradianceLUT, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.hdrColor, ImageLayout::General);

		const PipelineResourceCache* fogPipeline = resourceManager->GetPipelineData(
			useRtPipeline ? "Atmosphere::VolumetricFogRT" : "Atmosphere::VolumetricFogNoRT"
		);
		const std::array<Handle, 2> sets = {ctx.globalDescriptorSet, fogSet};

		uint32_t flags = 0;
		if (settings.fogUseBlueNoise)
			flags |= kFogFlagBlueNoise;
		if (useRtPipeline && settings.fogUseRTShadows)
			flags |= kFogFlagRTShadows;
		if (settings.fogRestrictToSponza)
			flags |= kFogFlagRestrictToBox;
		if (settings.enableVolumetricShadows)
			flags |= kFogFlagVolumetricShadows;

		VolumetricFogPushConstants push{};
		push.density = settings.enableVolumetricFog ? settings.fogDensity : 0.0f;
		push.heightFalloff = settings.fogHeightFalloff;
		push.baseHeight = settings.fogBaseHeight;
		push.maxDistance = settings.fogMaxDistance;
		push.anisotropy = settings.fogAnisotropy;
		push.noiseStrength = settings.fogNoiseStrength;
		push.stepCount = std::clamp(settings.fogStepCount, 8u, 48u);
		push.flags = flags;
		// Sponza world-space bounding box (at scale 0.01f: X[-19.2, 18.0], Y[-0.5, 14.3], Z[-11.8, 11.1])
		push.boxMin[0] = -19.2f;
		push.boxMin[1] = -0.5f;
		push.boxMin[2] = -11.8f;
		push.boxMin[3] = 2.2f;
		push.boxMax[0] = 18.0f;
		push.boxMax[1] = 14.3f;
		push.boxMax[2] = 11.1f;
		push.boxMax[3] = settings.enableVolumetricShadows ? settings.volumetricShadowIntensity : 0.0f;

		const uint32_t groupsX = (ctx.viewportWidth + 7u) / 8u;
		const uint32_t groupsY = (ctx.viewportHeight + 7u) / 8u;

		renderingDevice->BindComputePipeline(fogPipeline->pipeline);
		renderingDevice->BindComputeDescriptorSets(fogPipeline->layoutName, 0, sets);
		renderingDevice->PushComputeConstants(fogPipeline->layoutName, &push, sizeof(push));
		renderingDevice->Dispatch(groupsX, groupsY, 1);
	}
}
