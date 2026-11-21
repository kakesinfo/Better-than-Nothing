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
		RenderingDevice* renderingDevice = ctx.renderingDevice;
		ResourceManager* resourceManager = ctx.resourceManager;

		const uint32_t readIdx = ctx.temporalReadIndex;
		const uint32_t writeIdx = ctx.temporalWriteIndex;

		// 1. Temporal Anti-Aliasing (TAA) resolve in HDR space
		renderingDevice->TransitionTexture(ctx.hdrColor, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.taaHistory[readIdx], ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.gbufferMotion, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.depthTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.taaHistory[writeIdx], ImageLayout::General);

		const PipelineResourceCache* taaPipeline = resourceManager->GetPipelineData("PostProcess::TAA");
		const std::array<Handle, 2>	 taaSets = {ctx.globalDescriptorSet, ctx.taaDescriptorSets[writeIdx]};

		const uint32_t groupsX = (ctx.viewportWidth + 7u) / 8u;
		const uint32_t groupsY = (ctx.viewportHeight + 7u) / 8u;

		renderingDevice->BindComputePipeline(taaPipeline->pipeline);
		renderingDevice->BindComputeDescriptorSets(taaPipeline->layoutName, 0, taaSets);
		renderingDevice->Dispatch(groupsX, groupsY, 1);

		// 2. Tone Mapping (AgX / ACES / Reinhard) + sRGB output to SwapChain
		renderingDevice->TransitionTexture(ctx.taaHistory[writeIdx], ImageLayout::ShaderReadOnly);

		ColorAttachmentDesc swapColor{};
		swapColor.useSwapChainImage = true;
		swapColor.swapChainImageIndex = ctx.swapChainImageIndex;
		swapColor.loadOp = LoadOp::DontCare;
		swapColor.storeOp = StoreOp::Store;

		RenderPassDescriptor passDesc{};
		passDesc.colorAttachments = {swapColor};
		passDesc.width = ctx.viewportWidth;
		passDesc.height = ctx.viewportHeight;

		const PipelineResourceCache* toneMapPipeline = resourceManager->GetPipelineData("PostProcess::ToneMap");
		const std::array<Handle, 2>	 toneMapSets = {ctx.globalDescriptorSet, ctx.toneMapDescriptorSets[writeIdx]};

		renderingDevice->BeginRenderPass(passDesc);
		renderingDevice->DrawFullscreen(toneMapPipeline->pipeline, toneMapPipeline->layoutName, toneMapSets);
		renderingDevice->EndRenderPass();
	}
}
