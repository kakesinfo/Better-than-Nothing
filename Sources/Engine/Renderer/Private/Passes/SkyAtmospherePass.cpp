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

#include "SkyAtmospherePass.hpp"

#include <Renderer/RenderSettings.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>
#include <array>
#include <cmath>

namespace BetterThanNothing
{
	void SkyAtmospherePass::EvaluateDirtyFlags(SkyLutDirtyState& state, const Vector3& lightDirection, const float skyLuminanceNits)
	{
		if (state.lastSkyLuminanceNits < 0.0f ||
			Vector3::Distance2(state.lastLightDirection, lightDirection) > 1e-8f ||
			std::abs(state.lastSkyLuminanceNits - skyLuminanceNits) > 1e-3f)
		{
			state.skyIblLutsDirty = true;
			state.lastLightDirection = lightDirection;
			state.lastSkyLuminanceNits = skyLuminanceNits;
		}
	}

	void SkyAtmospherePass::Execute(RenderPassContext& ctx)
	{
		if (!ctx.renderingDevice || !ctx.hdrColor.IsValid() || !ctx.skyRenderDescriptorSet.IsValid())
			return;

		const auto&		 settings = RenderSettings::Get();
		RenderingDevice* renderingDevice = ctx.renderingDevice;

		EvaluateDirtyFlags(m_dirtyState, ctx.lightDirection, settings.skyLuminanceNits);

		if (m_dirtyState.brdfLutDirty && ctx.brdfLUT.IsValid())
		{
			renderingDevice->TransitionTexture(ctx.brdfLUT, ImageLayout::General);
			const PipelineResourceCache* pipeline = ctx.resourceManager->GetPipelineData("Sky::BrdfLUT");
			renderingDevice->BindComputePipeline(pipeline->pipeline);
			const std::array<Handle, 1> sets = {ctx.skyBrdfDescriptorSet};
			renderingDevice->BindComputeDescriptorSets("Sky::BrdfLUT", 0, sets);
			renderingDevice->Dispatch(256u / 8u, 256u / 8u, 1u);
			renderingDevice->ComputeBarrier();
			renderingDevice->TransitionTexture(ctx.brdfLUT, ImageLayout::ShaderReadOnly);
			m_dirtyState.brdfLutDirty = false;
		}

		if (m_dirtyState.atmosphereStaticLutsDirty && ctx.transmittanceLUT.IsValid() && ctx.multiScatteringLUT.IsValid())
		{
			renderingDevice->TransitionTexture(ctx.transmittanceLUT, ImageLayout::General);
			const PipelineResourceCache* transPipe = ctx.resourceManager->GetPipelineData("Sky::TransmittanceLUT");
			renderingDevice->BindComputePipeline(transPipe->pipeline);
			const std::array<Handle, 1> transSets = {ctx.skyTransmittanceDescriptorSet};
			renderingDevice->BindComputeDescriptorSets("Sky::TransmittanceLUT", 0, transSets);
			renderingDevice->Dispatch(256u / 8u, 64u / 8u, 1u);
			renderingDevice->ComputeBarrier();
			renderingDevice->TransitionTexture(ctx.transmittanceLUT, ImageLayout::ShaderReadOnly);

			renderingDevice->TransitionTexture(ctx.multiScatteringLUT, ImageLayout::General);
			const PipelineResourceCache* msPipe = ctx.resourceManager->GetPipelineData("Sky::MultiScatteringLUT");
			renderingDevice->BindComputePipeline(msPipe->pipeline);
			const std::array<Handle, 1> msSets = {ctx.skyMultiScatteringDescriptorSet};
			renderingDevice->BindComputeDescriptorSets("Sky::MultiScatteringLUT", 0, msSets);
			renderingDevice->Dispatch(32u / 8u, 32u / 8u, 1u);
			renderingDevice->ComputeBarrier();
			renderingDevice->TransitionTexture(ctx.multiScatteringLUT, ImageLayout::ShaderReadOnly);

			m_dirtyState.atmosphereStaticLutsDirty = false;
		}

		if (m_dirtyState.skyIblLutsDirty && ctx.skyViewLUT.IsValid() && ctx.irradianceLUT.IsValid() && ctx.prefilteredEnvLUT.IsValid())
		{
			renderingDevice->TransitionTexture(ctx.skyViewLUT, ImageLayout::General);
			const PipelineResourceCache* skyViewPipe = ctx.resourceManager->GetPipelineData("Sky::SkyViewLUT");
			renderingDevice->BindComputePipeline(skyViewPipe->pipeline);
			const std::array<Handle, 2> skyViewSets = {ctx.globalDescriptorSet, ctx.skyViewDescriptorSet};
			renderingDevice->BindComputeDescriptorSets("Sky::SkyViewLUT", 0, skyViewSets);
			SkyViewPushConstants viewPc{};
			viewPc.skyLuminanceNits = settings.skyLuminanceNits;
			renderingDevice->PushComputeConstants("Sky::SkyViewLUT", &viewPc, sizeof(viewPc));
			renderingDevice->Dispatch((192u + 7u) / 8u, (108u + 7u) / 8u, 1u);
			renderingDevice->ComputeBarrier();
			renderingDevice->TransitionTexture(ctx.skyViewLUT, ImageLayout::ShaderReadOnly);

			renderingDevice->TransitionTexture(ctx.irradianceLUT, ImageLayout::General);
			const PipelineResourceCache* irrPipe = ctx.resourceManager->GetPipelineData("Sky::IrradianceConvolve");
			renderingDevice->BindComputePipeline(irrPipe->pipeline);
			const std::array<Handle, 1> irrSets = {ctx.skyIrradianceDescriptorSet};
			renderingDevice->BindComputeDescriptorSets("Sky::IrradianceConvolve", 0, irrSets);
			renderingDevice->Dispatch(64u / 8u, 32u / 8u, 1u);
			renderingDevice->ComputeBarrier();
			renderingDevice->TransitionTexture(ctx.irradianceLUT, ImageLayout::ShaderReadOnly);

			renderingDevice->TransitionTexture(ctx.prefilteredEnvLUT, ImageLayout::General);
			const PipelineResourceCache* prefPipe = ctx.resourceManager->GetPipelineData("Sky::PrefilterEnv");
			renderingDevice->BindComputePipeline(prefPipe->pipeline);
			const std::array<Handle, 1> prefSets = {ctx.skyPrefilterDescriptorSet};
			renderingDevice->BindComputeDescriptorSets("Sky::PrefilterEnv", 0, prefSets);
			renderingDevice->Dispatch(128u / 8u, 320u / 8u, 1u);
			renderingDevice->ComputeBarrier();
			renderingDevice->TransitionTexture(ctx.prefilteredEnvLUT, ImageLayout::ShaderReadOnly);

			m_dirtyState.skyIblLutsDirty = false;
		}

		// Render sky, solar disk & volumetric clouds into hdrColor where depth >= 1.0
		const uint32_t groupX = (ctx.viewportWidth + 7u) / 8u;
		const uint32_t groupY = (ctx.viewportHeight + 7u) / 8u;
		if (groupX > 0u && groupY > 0u)
		{
			renderingDevice->TransitionTexture(ctx.depthTarget, ImageLayout::ShaderReadOnly);
			renderingDevice->TransitionTexture(ctx.hdrColor, ImageLayout::General);

			const PipelineResourceCache* renderPipe = ctx.resourceManager->GetPipelineData("Sky::Render");
			renderingDevice->BindComputePipeline(renderPipe->pipeline);
			const std::array<Handle, 2> renderSets = {ctx.globalDescriptorSet, ctx.skyRenderDescriptorSet};
			renderingDevice->BindComputeDescriptorSets("Sky::Render", 0, renderSets);

			SkyRenderPushConstants renderPc{};
			renderPc.skyLuminanceNits = settings.skyLuminanceNits;
			renderPc.cloudCoverage = settings.cloudCoverage;
			renderPc.cloudSpeed = settings.cloudSpeed;
			renderPc.enableClouds = settings.enableClouds ? 1u : 0u;
			renderingDevice->PushComputeConstants("Sky::Render", &renderPc, sizeof(renderPc));

			renderingDevice->Dispatch(groupX, groupY, 1u);
			renderingDevice->ComputeBarrier();
		}
	}
}
