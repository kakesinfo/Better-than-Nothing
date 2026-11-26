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

#include "PostProcessPass.hpp"

#include <RenderingDevice/RenderPassDescription.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>
#include <array>

namespace BetterThanNothing
{
	void PostProcessPass::Execute(RenderPassContext& ctx)
	{
		if (!ctx.hdrColor.IsValid() || !ctx.taaResolved.IsValid() || !ctx.taaDescriptorSet.IsValid() || !ctx.tonemapDescriptorSet.IsValid())
			return;

		RenderingDevice* renderingDevice = ctx.renderingDevice;

		renderingDevice->TransitionTexture(ctx.hdrColor, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.gbufferMotion, ImageLayout::ShaderReadOnly);
		if (ctx.taaHistoryRead.IsValid())
			renderingDevice->TransitionTexture(ctx.taaHistoryRead, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.taaResolved, ImageLayout::General);

		const PipelineResourceCache* taaPipeline = ctx.resourceManager->GetPipelineData("PostProcess::TAA");
		renderingDevice->BindComputePipeline(taaPipeline->pipeline);

		const std::array<Handle, 2> taaSets = {ctx.globalDescriptorSet, ctx.taaDescriptorSet};
		renderingDevice->BindComputeDescriptorSets("PostProcess::TAA", 0, taaSets);

		const uint32_t groupX = (ctx.viewportWidth + 7) / 8;
		const uint32_t groupY = (ctx.viewportHeight + 7) / 8;
		renderingDevice->Dispatch(groupX, groupY, 1);

		renderingDevice->TransitionTexture(ctx.taaResolved, ImageLayout::ShaderReadOnly);

		ColorAttachmentDesc swapAttachment{};
		swapAttachment.useSwapChainImage = true;
		swapAttachment.swapChainImageIndex = ctx.swapChainImageIndex;
		swapAttachment.loadOp = LoadOp::DontCare;
		swapAttachment.storeOp = StoreOp::Store;

		RenderPassDescriptor passDesc{};
		passDesc.colorAttachments = {swapAttachment};
		passDesc.width = ctx.viewportWidth;
		passDesc.height = ctx.viewportHeight;

		const PipelineResourceCache* tonemapPipeline = ctx.resourceManager->GetPipelineData("PostProcess::ToneMap");
		const std::array<Handle, 2>	 tonemapSets = {ctx.globalDescriptorSet, ctx.tonemapDescriptorSet};

		renderingDevice->BeginRenderPass(passDesc);
		renderingDevice->DrawFullscreenTriangle(tonemapPipeline->pipeline, "PostProcess::ToneMap", tonemapSets);
		renderingDevice->EndRenderPass();
	}
}
