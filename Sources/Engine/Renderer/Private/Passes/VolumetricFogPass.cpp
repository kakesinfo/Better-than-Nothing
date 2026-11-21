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
	}

	void VolumetricFogPass::Execute(RenderPassContext& ctx)
	{
		const auto& settings = RenderSettings::Get();
		if (!settings.enableVolumetricFog && settings.debugView != EDebugViewMode::VolumetricFog)
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

		VolumetricFogPushConstants push{};
		push.density = settings.fogDensity;
		push.heightFalloff = settings.fogHeightFalloff;
		push.baseHeight = settings.fogBaseHeight;
		push.maxDistance = settings.fogMaxDistance;
		push.anisotropy = settings.fogAnisotropy;
		push.noiseStrength = settings.fogNoiseStrength;
		push.stepCount = std::clamp(settings.fogStepCount, 8u, 64u);
		push.flags = flags;

		const uint32_t groupsX = (ctx.viewportWidth + 7u) / 8u;
		const uint32_t groupsY = (ctx.viewportHeight + 7u) / 8u;

		renderingDevice->BindComputePipeline(fogPipeline->pipeline);
		renderingDevice->BindComputeDescriptorSets(fogPipeline->layoutName, 0, sets);
		renderingDevice->PushComputeConstants(fogPipeline->layoutName, &push, sizeof(push));
		renderingDevice->Dispatch(groupsX, groupsY, 1);
	}
}
