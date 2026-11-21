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

#include "AmbientOcclusionPass.hpp"

#include <Common/RenderSettings.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>
#include <array>

namespace BetterThanNothing
{
	struct AoDenoisePushConstants
	{
		int32_t stepSize = 0; // 0 = Temporal Accumulation + Spatial Moments, >0 = A-Trous stride (1, 2, 4)
		float	depthSigma = 0.015f;
		float	normalSigma = 16.0f;
		float	variancePhi = 4.0f;
		float	stabilityBias = 0.08f;
		float	radianceClipStdK = 1.5f;
		float	disocclusionThreshold = 0.025f;
		float	_pad = 0.0f;
	};

	void AmbientOcclusionPass::Execute(RenderPassContext& ctx)
	{
		if (ctx.activeAoMode == EAOMode::Off)
			return;

		RenderingDevice* renderingDevice = ctx.renderingDevice;
		ResourceManager* resourceManager = ctx.resourceManager;
		const auto&		 settings = RenderSettings::Get();

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

		renderingDevice->TransitionTexture(ctx.depthTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.gbufferNormal, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.aoRaw, ImageLayout::General);

		// 1. Compute raw AO via HW Ray Query (RTAO) or Screen-Space Ground-Truth AO (GTAO)
		const bool useRtao = (ctx.activeAoMode == EAOMode::RTAO && ctx.rtSupported && ctx.tlasValid && ctx.rtaoDescriptorSet.IsValid());
		if (useRtao)
		{
			const PipelineResourceCache* rtaoPipeline = resourceManager->GetPipelineData("AO::RTAO");
			const std::array<Handle, 2>	 rtaoSets = {ctx.globalDescriptorSet, ctx.rtaoDescriptorSet};
			renderingDevice->BindComputePipeline(rtaoPipeline->pipeline);
			renderingDevice->BindComputeDescriptorSets(rtaoPipeline->layoutName, 0, rtaoSets);
			renderingDevice->Dispatch(groupsX, groupsY, 1);
		}
		else
		{
			const PipelineResourceCache* gtaoPipeline = resourceManager->GetPipelineData("AO::GTAO");
			const std::array<Handle, 2>	 gtaoSets = {ctx.globalDescriptorSet, ctx.gtaoDescriptorSet};
			renderingDevice->BindComputePipeline(gtaoPipeline->pipeline);
			renderingDevice->BindComputeDescriptorSets(gtaoPipeline->layoutName, 0, gtaoSets);
			renderingDevice->Dispatch(groupsX, groupsY, 1);
		}

		// 2. Stage 1 (stepSize == 0): 5x5 Cross-Bilateral Spatial Moments + Temporal Accumulation -> aoPing
		renderingDevice->TransitionTexture(ctx.aoRaw, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.aoHistory, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.prevDepthTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.prevNormalTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.gbufferMotion, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.aoPing, ImageLayout::General);

		const PipelineResourceCache* denoisePipeline = resourceManager->GetPipelineData("AO::Denoise");
		renderingDevice->BindComputePipeline(denoisePipeline->pipeline);

		{
			const std::array<Handle, 2> denoiseSets = {ctx.globalDescriptorSet, ctx.aoDenoiseDescriptorSet};
			renderingDevice->BindComputeDescriptorSets(denoisePipeline->layoutName, 0, denoiseSets);

			AoDenoisePushConstants temporalPush{};
			temporalPush.stepSize = 0;
			temporalPush.depthSigma = settings.denoiserDepthSigma;
			temporalPush.normalSigma = settings.denoiserNormalSigma;
			temporalPush.variancePhi = settings.denoiserShadowPhi;
			temporalPush.stabilityBias = settings.denoiserTemporalStabilityBias;
			temporalPush.radianceClipStdK = settings.denoiserRadianceClipStdK;
			temporalPush.disocclusionThreshold = settings.denoiserDisocclusionThreshold;
			renderingDevice->PushComputeConstants(denoisePipeline->layoutName, &temporalPush, sizeof(temporalPush));
			renderingDevice->Dispatch(groupsX, groupsY, 1);
		}

		// 3. Stage 2 (stepSize == 1, 2, 4): 3-pass Variance-Guided A-Trous Wavelet Edge-Stopping Filter
		static constexpr std::array<int32_t, 3> kStepSizes = {1, 2, 4};
		const std::array<Handle, 3>				srcTargets = {ctx.aoPing, ctx.aoPong, ctx.aoPing};
		const std::array<Handle, 3>				dstTargets = {ctx.aoPong, ctx.aoPing, ctx.aoDenoised};

		for (uint32_t passIdx = 0; passIdx < 3; ++passIdx)
		{
			renderingDevice->TransitionTexture(srcTargets[passIdx], ImageLayout::ShaderReadOnly);
			renderingDevice->TransitionTexture(dstTargets[passIdx], ImageLayout::General);

			const std::array<Handle, 2> filterSets = {ctx.globalDescriptorSet, ctx.aoFilterDescriptorSets[passIdx]};
			renderingDevice->BindComputeDescriptorSets(denoisePipeline->layoutName, 0, filterSets);

			AoDenoisePushConstants filterPush{};
			filterPush.stepSize = kStepSizes[passIdx];
			filterPush.depthSigma = settings.denoiserDepthSigma;
			filterPush.normalSigma = settings.denoiserNormalSigma;
			filterPush.variancePhi = settings.denoiserShadowPhi;
			filterPush.stabilityBias = settings.denoiserTemporalStabilityBias;
			filterPush.radianceClipStdK = settings.denoiserRadianceClipStdK;
			filterPush.disocclusionThreshold = settings.denoiserDisocclusionThreshold;
			renderingDevice->PushComputeConstants(denoisePipeline->layoutName, &filterPush, sizeof(filterPush));
			renderingDevice->Dispatch(groupsX, groupsY, 1);

			// Snapshot temporal history after the first A-Trous pass (step=1) for crisp yet low-variance temporal feedback
			if (passIdx == 0)
				renderingDevice->CopyTexture(ctx.aoPong, ctx.aoHistory);
		}
	}
}
