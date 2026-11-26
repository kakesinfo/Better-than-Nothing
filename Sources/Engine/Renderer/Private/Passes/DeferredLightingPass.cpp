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

#include "DeferredLightingPass.hpp"

#include <RenderingDevice/RenderingDevice.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>
#include <array>

namespace BetterThanNothing
{
	void DeferredLightingPass::Execute(RenderPassContext& ctx)
	{
		if (!ctx.hdrColor.IsValid() || !ctx.deferredLightingDescriptorSet.IsValid())
			return;

		RenderingDevice* renderingDevice = ctx.renderingDevice;

		renderingDevice->TransitionTexture(ctx.gbufferAlbedo, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.gbufferNormal, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.gbufferMaterial, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.gbufferMotion, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.depthTarget, ImageLayout::ShaderReadOnly);
		if (ctx.rtShadowDenoised.IsValid())
			renderingDevice->TransitionTexture(ctx.rtShadowDenoised, ImageLayout::ShaderReadOnly);
		if (ctx.sssTarget.IsValid())
			renderingDevice->TransitionTexture(ctx.sssTarget, ImageLayout::ShaderReadOnly);
		if (ctx.aoDenoised.IsValid())
			renderingDevice->TransitionTexture(ctx.aoDenoised, ImageLayout::ShaderReadOnly);
		if (ctx.transmittanceLUT.IsValid())
			renderingDevice->TransitionTexture(ctx.transmittanceLUT, ImageLayout::ShaderReadOnly);
		if (ctx.irradianceLUT.IsValid())
			renderingDevice->TransitionTexture(ctx.irradianceLUT, ImageLayout::ShaderReadOnly);
		if (ctx.prefilteredEnvLUT.IsValid())
			renderingDevice->TransitionTexture(ctx.prefilteredEnvLUT, ImageLayout::ShaderReadOnly);
		if (ctx.brdfLUT.IsValid())
			renderingDevice->TransitionTexture(ctx.brdfLUT, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.hdrColor, ImageLayout::General);

		const PipelineResourceCache* pipeline = ctx.resourceManager->GetPipelineData("Deferred::Lighting");
		renderingDevice->BindComputePipeline(pipeline->pipeline);

		const std::array<Handle, 2> sets = {ctx.globalDescriptorSet, ctx.deferredLightingDescriptorSet};
		renderingDevice->BindComputeDescriptorSets("DeferredLighting", 0, sets);

		const uint32_t groupX = (ctx.viewportWidth + 7) / 8;
		const uint32_t groupY = (ctx.viewportHeight + 7) / 8;
		renderingDevice->Dispatch(groupX, groupY, 1);
	}
}
