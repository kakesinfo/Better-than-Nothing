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

#include <RenderingDevice/RenderingDevice.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>
#include <array>

namespace BetterThanNothing
{
	void AmbientOcclusionPass::Execute(RenderPassContext& ctx)
	{
		if (ctx.activeAoMode == EAOMode::Off)
			return;

		RenderingDevice* renderingDevice = ctx.renderingDevice;
		ResourceManager* resourceManager = ctx.resourceManager;

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

		// 2. Spatio-temporal edge-aware bilateral denoise
		renderingDevice->TransitionTexture(ctx.aoRaw, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.aoHistory, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.prevDepthTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.prevNormalTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.gbufferMotion, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.aoDenoised, ImageLayout::General);

		const PipelineResourceCache* denoisePipeline = resourceManager->GetPipelineData("AO::Denoise");
		const std::array<Handle, 2>	 denoiseSets = {ctx.globalDescriptorSet, ctx.aoDenoiseDescriptorSet};
		renderingDevice->BindComputePipeline(denoisePipeline->pipeline);
		renderingDevice->BindComputeDescriptorSets(denoisePipeline->layoutName, 0, denoiseSets);
		renderingDevice->Dispatch(groupsX, groupsY, 1);

		// 3. Update temporal AO history
		renderingDevice->CopyTexture(ctx.aoDenoised, ctx.aoHistory);
	}
}
