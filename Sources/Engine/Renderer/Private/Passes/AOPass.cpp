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

#include "AOPass.hpp"

#include <RenderingDevice/RenderingDevice.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>
#include <array>

namespace BetterThanNothing
{
	EAOMode AOPass::ResolveEffectiveMode(const EAOMode requested, const bool rayQuerySupported, const bool hasTlas)
	{
		if (requested == EAOMode::Off)
			return EAOMode::Off;
		if (requested == EAOMode::RTAO && rayQuerySupported && hasTlas)
			return EAOMode::RTAO;
		return EAOMode::GTAO;
	}

	std::pair<uint32_t, uint32_t> AOPass::ComputeDispatchGroups(const uint32_t width, const uint32_t height)
	{
		return {(width + 7u) / 8u, (height + 7u) / 8u};
	}

	void AOPass::Execute(RenderPassContext& ctx)
	{
		const auto& settings = RenderSettings::Get();
		if (!ctx.renderingDevice || !ctx.aoRaw.IsValid() || !ctx.aoDenoised.IsValid() || !ctx.aoHistory.IsValid())
			return;

		const EAOMode effectiveMode = ResolveEffectiveMode(settings.aoMode, ctx.renderingDevice->IsRayQuerySupported(), ctx.topLevelAS.IsValid());
		if (effectiveMode == EAOMode::Off)
			return;

		const auto [groupX, groupY] = ComputeDispatchGroups(ctx.viewportWidth, ctx.viewportHeight);
		if (groupX == 0 || groupY == 0)
			return;

		RenderingDevice* renderingDevice = ctx.renderingDevice;
		renderingDevice->TransitionTexture(ctx.depthTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.gbufferNormal, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.aoRaw, ImageLayout::General);

		AOPushConstants pc{};
		pc.aoRadius = settings.aoRadius;
		pc.aoStrength = settings.aoStrength;
		pc.frameIndex = ctx.frameIndex;

		if (effectiveMode == EAOMode::RTAO)
		{
			renderingDevice->UpdateDescriptorAccelerationStructure(ctx.rtaoDescriptorSet, 2, ctx.topLevelAS);
			const PipelineResourceCache* pipeline = ctx.resourceManager->GetPipelineData("AO::RTAO");
			renderingDevice->BindComputePipeline(pipeline->pipeline);
			const std::array<Handle, 2> sets = {ctx.globalDescriptorSet, ctx.rtaoDescriptorSet};
			renderingDevice->BindComputeDescriptorSets("AO::RTAO", 0, sets);
			renderingDevice->PushComputeConstants("AO::RTAO", &pc, sizeof(pc));
		}
		else
		{
			const PipelineResourceCache* pipeline = ctx.resourceManager->GetPipelineData("AO::GTAO");
			renderingDevice->BindComputePipeline(pipeline->pipeline);
			const std::array<Handle, 2> sets = {ctx.globalDescriptorSet, ctx.gtaoDescriptorSet};
			renderingDevice->BindComputeDescriptorSets("AO::GTAO", 0, sets);
			renderingDevice->PushComputeConstants("AO::GTAO", &pc, sizeof(pc));
		}

		renderingDevice->Dispatch(groupX, groupY, 1);
		renderingDevice->ComputeBarrier();

		renderingDevice->TransitionTexture(ctx.aoRaw, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.aoHistory, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.prevDepthTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.prevNormalTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.gbufferMotion, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.aoDenoised, ImageLayout::General);

		const PipelineResourceCache* denoisePipeline = ctx.resourceManager->GetPipelineData("AO::Denoise");
		renderingDevice->BindComputePipeline(denoisePipeline->pipeline);
		const std::array<Handle, 2> denoiseSets = {ctx.globalDescriptorSet, ctx.aoDenoiseDescriptorSet};
		renderingDevice->BindComputeDescriptorSets("AO::Denoise", 0, denoiseSets);
		renderingDevice->Dispatch(groupX, groupY, 1);
		renderingDevice->ComputeBarrier();

		renderingDevice->CopyTexture(ctx.aoDenoised, ctx.aoHistory);
		renderingDevice->TransitionTexture(ctx.aoDenoised, ImageLayout::ShaderReadOnly);
	}
}
