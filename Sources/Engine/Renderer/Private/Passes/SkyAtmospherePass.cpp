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

#include "SkyAtmospherePass.hpp"

#include <RenderingDevice/RenderingDevice.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>
#include <array>

namespace BetterThanNothing
{
	void SkyAtmospherePass::Execute(RenderPassContext& ctx)
	{
		RenderingDevice* renderingDevice = ctx.renderingDevice;
		ResourceManager* resourceManager = ctx.resourceManager;

		// 0. Precompute Split-Sum GGX BRDF Integration LUT (256x256) once on startup
		if (ctx.brdfLutDirty)
		{
			renderingDevice->TransitionTexture(ctx.brdfLUT, ImageLayout::General);
			const PipelineResourceCache* brdfPipeline = resourceManager->GetPipelineData("Sky::BrdfLUT");
			const std::array<Handle, 1>	 brdfSets = {ctx.skyBrdfDescriptorSet};
			renderingDevice->BindComputePipeline(brdfPipeline->pipeline);
			renderingDevice->BindComputeDescriptorSets(brdfPipeline->layoutName, 0, brdfSets);
			renderingDevice->Dispatch((kBrdfLutSize + 7u) / 8u, (kBrdfLutSize + 7u) / 8u, 1);
			renderingDevice->TransitionTexture(ctx.brdfLUT, ImageLayout::ShaderReadOnly);
		}

		// 1 & 2. Recompute Transmittance LUT (256x64) and Multi-Scattering LUT (32x32) when dirty
		if (ctx.atmosphereStaticLutsDirty)
		{
			renderingDevice->TransitionTexture(ctx.transmittanceLUT, ImageLayout::General);
			const PipelineResourceCache* transPipeline = resourceManager->GetPipelineData("Sky::TransmittanceLUT");
			const std::array<Handle, 2>	 transSets = {ctx.globalDescriptorSet, ctx.skyTransmittanceDescriptorSet};
			renderingDevice->BindComputePipeline(transPipeline->pipeline);
			renderingDevice->BindComputeDescriptorSets(transPipeline->layoutName, 0, transSets);
			renderingDevice->Dispatch((kTransmittanceWidth + 7u) / 8u, (kTransmittanceHeight + 7u) / 8u, 1);

			renderingDevice->TransitionTexture(ctx.transmittanceLUT, ImageLayout::ShaderReadOnly);
			renderingDevice->TransitionTexture(ctx.multiScatteringLUT, ImageLayout::General);
			const PipelineResourceCache* msPipeline = resourceManager->GetPipelineData("Sky::MultiScatteringLUT");
			const std::array<Handle, 2>	 msSets = {ctx.globalDescriptorSet, ctx.skyMultiScatteringDescriptorSet};
			renderingDevice->BindComputePipeline(msPipeline->pipeline);
			renderingDevice->BindComputeDescriptorSets(msPipeline->layoutName, 0, msSets);
			renderingDevice->Dispatch((kMultiScatteringSize + 7u) / 8u, (kMultiScatteringSize + 7u) / 8u, 1);

			renderingDevice->TransitionTexture(ctx.multiScatteringLUT, ImageLayout::ShaderReadOnly);
		}

		// 3. Recompute Sky-View LUT (192x108) dynamically every frame for current sun direction & camera altitude
		renderingDevice->TransitionTexture(ctx.transmittanceLUT, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.multiScatteringLUT, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.skyViewLUT, ImageLayout::General);

		const PipelineResourceCache* skyViewPipeline = resourceManager->GetPipelineData("Sky::SkyViewLUT");
		const std::array<Handle, 2>	 skyViewSets = {ctx.globalDescriptorSet, ctx.skyViewDescriptorSet};
		renderingDevice->BindComputePipeline(skyViewPipeline->pipeline);
		renderingDevice->BindComputeDescriptorSets(skyViewPipeline->layoutName, 0, skyViewSets);
		renderingDevice->Dispatch((kSkyViewWidth + 7u) / 8u, (kSkyViewHeight + 7u) / 8u, 1);

		// 4. Convolve live Sky-View LUT into Diffuse Irradiance LUT (64x32) & GGX Prefiltered Specular Atlas (128x320)
		renderingDevice->TransitionTexture(ctx.skyViewLUT, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.irradianceLUT, ImageLayout::General);
		renderingDevice->TransitionTexture(ctx.prefilteredEnvLUT, ImageLayout::General);

		const PipelineResourceCache* irrPipeline = resourceManager->GetPipelineData("Sky::IrradianceConvolve");
		const std::array<Handle, 2>	 irrSets = {ctx.globalDescriptorSet, ctx.skyIrradianceDescriptorSet};
		renderingDevice->BindComputePipeline(irrPipeline->pipeline);
		renderingDevice->BindComputeDescriptorSets(irrPipeline->layoutName, 0, irrSets);
		renderingDevice->Dispatch((kIrradianceWidth + 7u) / 8u, (kIrradianceHeight + 7u) / 8u, 1);

		const PipelineResourceCache* prefPipeline = resourceManager->GetPipelineData("Sky::PrefilterEnv");
		const std::array<Handle, 2>	 prefSets = {ctx.globalDescriptorSet, ctx.skyPrefilterDescriptorSet};
		renderingDevice->BindComputePipeline(prefPipeline->pipeline);
		renderingDevice->BindComputeDescriptorSets(prefPipeline->layoutName, 0, prefSets);
		renderingDevice->Dispatch((kPrefilterWidth + 7u) / 8u, (kPrefilterHeight + 7u) / 8u, 1);

		// 5. Render full-resolution skybox + physical limb-darkened solar disk into hdrColor (depth >= 1.0)
		renderingDevice->TransitionTexture(ctx.depthTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.hdrColor, ImageLayout::General);

		const PipelineResourceCache* skyRenderPipeline = resourceManager->GetPipelineData("Sky::Render");
		const std::array<Handle, 2>	 skyRenderSets = {ctx.globalDescriptorSet, ctx.skyRenderDescriptorSet};
		const uint32_t				 groupsX = (ctx.viewportWidth + 7u) / 8u;
		const uint32_t				 groupsY = (ctx.viewportHeight + 7u) / 8u;

		renderingDevice->BindComputePipeline(skyRenderPipeline->pipeline);
		renderingDevice->BindComputeDescriptorSets(skyRenderPipeline->layoutName, 0, skyRenderSets);
		renderingDevice->Dispatch(groupsX, groupsY, 1);
	}
}
