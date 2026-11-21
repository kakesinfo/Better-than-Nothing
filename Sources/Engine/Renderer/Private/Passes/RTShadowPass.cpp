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

#include "RTShadowPass.hpp"

#include <Common/RenderSettings.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>
#include <array>

namespace BetterThanNothing
{
	struct FfxShadowTileClassPushConstants
	{
		float stabilityBias = 0.08f;
		float radianceClipStdK = 1.5f;
		float disocclusionThreshold = 0.025f;
		float _pad = 0.0f;
	};

	struct FfxShadowFilterPushConstants
	{
		int32_t stepSize = 1;
		float	depthSigma = 0.015f;
		float	normalSigma = 16.0f;
		float	shadowPhi = 4.0f;
	};

	void RTShadowPass::Execute(RenderPassContext& ctx)
	{
		if (!ctx.rtSupported || !ctx.rtShadowsActive || !ctx.rtShadowDescriptorSet.IsValid())
			return;

		RenderingDevice*  renderingDevice = ctx.renderingDevice;
		ResourceManager*  resourceManager = ctx.resourceManager;
		const auto&		  settings = RenderSettings::Get();

		FfxDenoiserSetupParams denoiserParams{};
		denoiserParams.maxRenderWidth = ctx.viewportWidth;
		denoiserParams.maxRenderHeight = ctx.viewportHeight;
		denoiserParams.crossBilateralNormalStrength = settings.denoiserNormalSigma;
		denoiserParams.stabilityBias = settings.denoiserTemporalStabilityBias;
		denoiserParams.radianceClipStdK = settings.denoiserRadianceClipStdK;
		denoiserParams.gaussianKernelRelaxation = settings.denoiserShadowPhi;
		denoiserParams.disocclusionThreshold = settings.denoiserDisocclusionThreshold;
		renderingDevice->ConfigureFfxDenoiser(denoiserParams);

		const uint32_t groupsX = (ctx.viewportWidth + 7u) / 8u;
		const uint32_t groupsY = (ctx.viewportHeight + 7u) / 8u;
		const uint32_t readIdx = ctx.temporalReadIndex;
		const uint32_t writeIdx = ctx.temporalWriteIndex;

		// 1. Hardware Ray Query shadow trace + AMD FFX 8x4 bitmask packing
		renderingDevice->TransitionTexture(ctx.depthTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.gbufferNormal, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.rtShadowRaw, ImageLayout::General);

		const PipelineResourceCache* tracePipeline = resourceManager->GetPipelineData("RTShadow::Trace");
		const std::array<Handle, 2>	 traceSets = {ctx.globalDescriptorSet, ctx.rtShadowDescriptorSet};
		renderingDevice->BindComputePipeline(tracePipeline->pipeline);
		renderingDevice->BindComputeDescriptorSets(tracePipeline->layoutName, 0, traceSets);
		renderingDevice->Dispatch(groupsX, groupsY, 1);

		// Ensure bitmask SSBO writes are visible to FFX Tile Classification
		renderingDevice->ComputeBarrier();

		// 2. AMD FidelityFX Shadow Denoiser - Stage 1: Tile Classification + Temporal Moments + Disocclusion Fixup
		renderingDevice->TransitionTexture(ctx.rtShadowRaw, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.rtShadowMoments[readIdx], ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.prevDepthTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.prevNormalTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.gbufferMotion, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.rtShadowMoments[writeIdx], ImageLayout::General);
		renderingDevice->TransitionTexture(ctx.rtShadowPing, ImageLayout::General);

		const PipelineResourceCache* tileClassPipeline = resourceManager->GetPipelineData("RTShadow::FFXTileClassification");
		const std::array<Handle, 2>	 tileClassSets = {ctx.globalDescriptorSet, ctx.ffxTileClassDescriptorSets[writeIdx]};
		renderingDevice->BindComputePipeline(tileClassPipeline->pipeline);
		renderingDevice->BindComputeDescriptorSets(tileClassPipeline->layoutName, 0, tileClassSets);

		FfxShadowTileClassPushConstants tilePush{};
		tilePush.stabilityBias = settings.denoiserTemporalStabilityBias;
		tilePush.radianceClipStdK = settings.denoiserRadianceClipStdK;
		tilePush.disocclusionThreshold = settings.denoiserDisocclusionThreshold;
		renderingDevice->PushComputeConstants(tileClassPipeline->layoutName, &tilePush, sizeof(tilePush));
		renderingDevice->Dispatch(groupsX, groupsY, 1);

		// Ensure tile classification metadata SSBO is visible to A-Trous filter passes
		renderingDevice->ComputeBarrier();

		// 3. AMD FidelityFX Shadow Denoiser - Stage 2: 3-pass Variance-Guided Edge-Stopping A-Trous Filter (step = 1, 2, 4)
		const PipelineResourceCache* filterPipeline = resourceManager->GetPipelineData("RTShadow::FFXFilterSoftShadows");
		renderingDevice->BindComputePipeline(filterPipeline->pipeline);

		static constexpr std::array<int32_t, 3> kStepSizes = {1, 2, 4};
		const std::array<Handle, 3>				srcTargets = {ctx.rtShadowPing, ctx.rtShadowPong, ctx.rtShadowPing};
		const std::array<Handle, 3>				dstTargets = {ctx.rtShadowPong, ctx.rtShadowPing, ctx.rtShadowDenoised};

		for (uint32_t passIdx = 0; passIdx < 3; ++passIdx)
		{
			renderingDevice->TransitionTexture(srcTargets[passIdx], ImageLayout::ShaderReadOnly);
			renderingDevice->TransitionTexture(dstTargets[passIdx], ImageLayout::General);

			const std::array<Handle, 2> filterSets = {ctx.globalDescriptorSet, ctx.ffxFilterDescriptorSets[passIdx]};
			renderingDevice->BindComputeDescriptorSets(filterPipeline->layoutName, 0, filterSets);

			FfxShadowFilterPushConstants push{};
			push.stepSize = kStepSizes[passIdx];
			push.depthSigma = settings.denoiserDepthSigma;
			push.normalSigma = settings.denoiserNormalSigma;
			push.shadowPhi = settings.denoiserShadowPhi;
			renderingDevice->PushComputeConstants(filterPipeline->layoutName, &push, sizeof(push));

			renderingDevice->Dispatch(groupsX, groupsY, 1);
		}
	}
}
