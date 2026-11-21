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
		RenderingDevice* renderingDevice = ctx.renderingDevice;
		ResourceManager* resourceManager = ctx.resourceManager;

		renderingDevice->TransitionTexture(ctx.gbufferAlbedo, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.gbufferNormal, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.gbufferMaterial, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.gbufferMotion, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.depthTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.rtShadowDenoised, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.screenSpaceShadow, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.transmittanceLUT, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.irradianceLUT, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.prefilteredEnvLUT, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.brdfLUT, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.aoDenoised, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.hdrColor, ImageLayout::General);

		const PipelineResourceCache* lightingPipeline = resourceManager->GetPipelineData("Deferred::Lighting");
		const std::array<Handle, 3>	 sets = {ctx.globalDescriptorSet, ctx.deferredLightingDescriptorSet, ctx.pointLightDescriptorSet};

		const uint32_t groupsX = (ctx.viewportWidth + 7u) / 8u;
		const uint32_t groupsY = (ctx.viewportHeight + 7u) / 8u;

		renderingDevice->BindComputePipeline(lightingPipeline->pipeline);
		renderingDevice->BindComputeDescriptorSets(lightingPipeline->layoutName, 0, sets);
		renderingDevice->Dispatch(groupsX, groupsY, 1);
	}
}
