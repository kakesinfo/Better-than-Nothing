/*
 *	Better than Nothing
 *	Copyright (C) 2026 kakesinfo
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "RTShadowPass.hpp"

#include <Renderer/RenderSettings.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>

namespace BetterThanNothing
{
	std::pair<uint32_t, uint32_t> RTShadowPass::ComputeDispatchGroups(const uint32_t width, const uint32_t height)
	{
		return {(width + 7u) / 8u, (height + 3u) / 4u};
	}

	void RTShadowPass::Execute(RenderPassContext& ctx)
	{
		const auto& settings = RenderSettings::Get();
		if (!settings.enableRTShadows || !ctx.renderingDevice || !ctx.renderingDevice->IsRayQuerySupported() || !ctx.topLevelAS.IsValid())
			return;
		if (!ctx.rtShadowRaw.IsValid() || !ctx.rtShadowPing.IsValid() || !ctx.rtShadowPong.IsValid() || !ctx.rtShadowDenoised.IsValid())
			return;

		RenderingDevice* renderingDevice = ctx.renderingDevice;
		const auto [groupX, groupY] = ComputeDispatchGroups(ctx.viewportWidth, ctx.viewportHeight);
		if (groupX == 0 || groupY == 0)
			return;

		renderingDevice->UpdateDescriptorAccelerationStructure(ctx.rtShadowTraceDescriptorSet, 2, ctx.topLevelAS);

		renderingDevice->TransitionTexture(ctx.depthTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.gbufferNormal, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.prevDepthTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.prevNormalTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.gbufferMotion, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.rtShadowMomentsRead, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.rtShadowRaw, ImageLayout::General);
		renderingDevice->TransitionTexture(ctx.rtShadowMomentsWrite, ImageLayout::General);
		renderingDevice->TransitionTexture(ctx.rtShadowPing, ImageLayout::General);

		// 1. Trace ray-queried shadows and pack 8x4 tile bitmask
		{
			const PipelineResourceCache* tracePipeline = ctx.resourceManager->GetPipelineData("RTShadow::Trace");
			renderingDevice->BindComputePipeline(tracePipeline->pipeline);

			const std::array<Handle, 2> sets = {ctx.globalDescriptorSet, ctx.rtShadowTraceDescriptorSet};
			renderingDevice->BindComputeDescriptorSets("RTShadow::Trace", 0, sets);

			RTShadowPushConstants pc{};
			pc.sunAngularRadius = settings.sunAngularRadius;
			pc.frameIndex = ctx.frameIndex;
			pc.enableRTShadows = settings.enableRTShadows ? 1u : 0u;
			renderingDevice->PushComputeConstants("RTShadow::Trace", &pc, sizeof(pc));

			renderingDevice->Dispatch(groupX, groupY, 1);
			renderingDevice->ComputeBarrier();
		}

		// 2. FFX Tile Classification & Temporal Moments
		{
			renderingDevice->TransitionTexture(ctx.rtShadowRaw, ImageLayout::ShaderReadOnly);

			const PipelineResourceCache* classifyPipeline = ctx.resourceManager->GetPipelineData("RTShadow::FFXTileClassification");
			renderingDevice->BindComputePipeline(classifyPipeline->pipeline);

			const std::array<Handle, 2> sets = {ctx.globalDescriptorSet, ctx.rtShadowClassifyDescriptorSet};
			renderingDevice->BindComputeDescriptorSets("RTShadow::FFXTileClassification", 0, sets);

			renderingDevice->Dispatch(groupX, groupY, 1);
			renderingDevice->ComputeBarrier();
		}

		// 3. FFX 3-pass A-Trous Spatial Filter: ping->pong (1), pong->ping (2), ping->denoised (4)
		const PipelineResourceCache* filterPipeline = ctx.resourceManager->GetPipelineData("RTShadow::FFXFilterSoftShadows");
		renderingDevice->BindComputePipeline(filterPipeline->pipeline);

		const std::array<Handle, 3> filterInputs = {ctx.rtShadowPing, ctx.rtShadowPong, ctx.rtShadowPing};
		const std::array<Handle, 3> filterOutputs = {ctx.rtShadowPong, ctx.rtShadowPing, ctx.rtShadowDenoised};
		constexpr auto				stepSizes = GetFilterStepSizes();

		for (size_t passIdx = 0; passIdx < stepSizes.size(); ++passIdx)
		{
			renderingDevice->TransitionTexture(filterInputs[passIdx], ImageLayout::ShaderReadOnly);
			renderingDevice->TransitionTexture(filterOutputs[passIdx], ImageLayout::General);

			const std::array<Handle, 2> sets = {ctx.globalDescriptorSet, ctx.rtShadowFilterDescriptorSets[passIdx]};
			renderingDevice->BindComputeDescriptorSets("RTShadow::FFXFilterSoftShadows", 0, sets);

			const int32_t stepSize = stepSizes[passIdx];
			renderingDevice->PushComputeConstants("RTShadow::FFXFilterSoftShadows", &stepSize, sizeof(stepSize));

			renderingDevice->Dispatch(groupX, groupY, 1);
			renderingDevice->ComputeBarrier();
		}

		renderingDevice->TransitionTexture(ctx.rtShadowDenoised, ImageLayout::ShaderReadOnly);
	}
}
