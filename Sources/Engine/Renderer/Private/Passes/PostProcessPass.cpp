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

#include <Common/RenderSettings.hpp>
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
		const auto&		 settings = RenderSettings::Get();

		const uint32_t readIdx = ctx.temporalReadIndex;
		const uint32_t writeIdx = ctx.temporalWriteIndex;
		const uint32_t displayW = ctx.displayWidth > 0 ? ctx.displayWidth : ctx.viewportWidth;
		const uint32_t displayH = ctx.displayHeight > 0 ? ctx.displayHeight : ctx.viewportHeight;

		bool fsrDispatched = false;
		if (ctx.fsrUpscaleActive && settings.debugView == EDebugViewMode::Final)
		{
			renderingDevice->ConfigureFfxUpscaler(settings.fsrVelocityFactor);

			FfxUpscaleDispatchParams fsrParams{};
			fsrParams.color = ctx.hdrColor;
			fsrParams.depth = ctx.depthTarget;
			fsrParams.motionVectors = ctx.gbufferMotion;
			fsrParams.output = ctx.taaHistory[writeIdx];
			fsrParams.renderWidth = ctx.viewportWidth;
			fsrParams.renderHeight = ctx.viewportHeight;
			fsrParams.upscaleWidth = displayW;
			fsrParams.upscaleHeight = displayH;
			fsrParams.jitterOffsetX = ctx.jitterPixelsX;
			fsrParams.jitterOffsetY = ctx.jitterPixelsY;
			// Our G-Buffer writes motion vectors as (currUV - prevUV); FSR expects (prevUV - currUV) * renderSize in pixels
			fsrParams.motionVectorScaleX = -static_cast<float>(ctx.viewportWidth);
			fsrParams.motionVectorScaleY = -static_cast<float>(ctx.viewportHeight);
			fsrParams.enableSharpening = settings.fsrEnableSharpening;
			fsrParams.sharpness = settings.fsrSharpness;
			fsrParams.frameTimeDeltaMs = ctx.frameTimeDeltaMs;
			fsrParams.preExposure = 1.0f;
			fsrParams.reset = ctx.fsrResetHistory;
			fsrParams.cameraNear = ctx.cameraNear;
			fsrParams.cameraFar = ctx.cameraFar;
			fsrParams.cameraFovAngleVerticalRadians = ctx.cameraFovVerticalRadians;
			fsrParams.viewSpaceToMetersFactor = 1.0f;
			fsrParams.drawDebugView = settings.fsrDrawDebugView;

			fsrDispatched = renderingDevice->DispatchFfxUpscale(fsrParams);
		}

		// Fallback to Native TAA / HDR resolve if FSR is not active or in G-Buffer debug view
		if (!fsrDispatched)
		{
			renderingDevice->TransitionTexture(ctx.hdrColor, ImageLayout::ShaderReadOnly);
			renderingDevice->TransitionTexture(ctx.taaHistory[readIdx], ImageLayout::ShaderReadOnly);
			renderingDevice->TransitionTexture(ctx.gbufferMotion, ImageLayout::ShaderReadOnly);
			renderingDevice->TransitionTexture(ctx.depthTarget, ImageLayout::ShaderReadOnly);
			renderingDevice->TransitionTexture(ctx.taaHistory[writeIdx], ImageLayout::General);

			const PipelineResourceCache* taaPipeline = resourceManager->GetPipelineData("PostProcess::TAA");
			const std::array<Handle, 2>	 taaSets = {ctx.globalDescriptorSet, ctx.taaDescriptorSets[writeIdx]};

			const uint32_t groupsX = (displayW + 7u) / 8u;
			const uint32_t groupsY = (displayH + 7u) / 8u;

			renderingDevice->BindComputePipeline(taaPipeline->pipeline);
			renderingDevice->BindComputeDescriptorSets(taaPipeline->layoutName, 0, taaSets);
			renderingDevice->Dispatch(groupsX, groupsY, 1);
		}

		// 1b. AMD FidelityFX Frame Generation (SDK v2.4.0: FSR Frame Gen 4.0.2 MLFI / 3.1.6 Optical Flow)
		bool fgDispatched = false;
		if (ctx.fgActive && settings.debugView == EDebugViewMode::Final && ctx.fgInterpolatedColor.IsValid())
		{
			FfxFrameGenDispatchParams fgParams{};
			fgParams.presentColor = ctx.taaHistory[writeIdx];
			fgParams.depth = ctx.depthTarget;
			fgParams.motionVectors = ctx.gbufferMotion;
			fgParams.outputInterpolated = ctx.fgInterpolatedColor;
			fgParams.renderWidth = ctx.viewportWidth;
			fgParams.renderHeight = ctx.viewportHeight;
			fgParams.displayWidth = displayW;
			fgParams.displayHeight = displayH;
			fgParams.jitterOffsetX = ctx.jitterPixelsX;
			fgParams.jitterOffsetY = ctx.jitterPixelsY;
			fgParams.motionVectorScaleX = -static_cast<float>(ctx.viewportWidth);
			fgParams.motionVectorScaleY = -static_cast<float>(ctx.viewportHeight);
			fgParams.frameTimeDeltaMs = ctx.frameTimeDeltaMs;
			fgParams.reset = ctx.fsrResetHistory;
			fgParams.cameraNear = ctx.cameraNear;
			fgParams.cameraFar = ctx.cameraFar;
			fgParams.cameraFovAngleVerticalRadians = ctx.cameraFovVerticalRadians;
			fgParams.viewSpaceToMetersFactor = 1.0f;
			fgParams.cameraPosition[0] = ctx.cameraPosition.GetX();
			fgParams.cameraPosition[1] = ctx.cameraPosition.GetY();
			fgParams.cameraPosition[2] = ctx.cameraPosition.GetZ();
			fgParams.cameraUp[0] = ctx.cameraUp.GetX();
			fgParams.cameraUp[1] = ctx.cameraUp.GetY();
			fgParams.cameraUp[2] = ctx.cameraUp.GetZ();
			fgParams.cameraRight[0] = ctx.cameraRight.GetX();
			fgParams.cameraRight[1] = ctx.cameraRight.GetY();
			fgParams.cameraRight[2] = ctx.cameraRight.GetZ();
			fgParams.cameraForward[0] = ctx.cameraForward.GetX();
			fgParams.cameraForward[1] = ctx.cameraForward.GetY();
			fgParams.cameraForward[2] = ctx.cameraForward.GetZ();
			fgParams.drawDebugTearLines = settings.fgDrawDebugTearLines;
			fgParams.drawDebugResetIndicators = settings.fgDrawDebugResetIndicators;
			fgParams.drawDebugPacingLines = settings.fgDrawDebugPacingLines;

			fgDispatched = renderingDevice->DispatchFfxFrameGen(fgParams);
		}

		// 2. Tone Mapping (AgX / ACES / Reinhard) + sRGB output to SwapChain at display resolution
		const bool presentGenerated = fgDispatched && (settings.fgShowInterpolatedFrame || settings.fgDrawDebugTearLines || settings.fgDrawDebugResetIndicators || settings.fgDrawDebugPacingLines);
		const Handle toneMapSourceTex = presentGenerated ? ctx.fgInterpolatedColor : ctx.taaHistory[writeIdx];
		const Handle toneMapSet = presentGenerated ? ctx.fgToneMapDescriptorSet : ctx.toneMapDescriptorSets[writeIdx];

		renderingDevice->TransitionTexture(toneMapSourceTex, ImageLayout::ShaderReadOnly);

		ColorAttachmentDesc swapColor{};
		swapColor.useSwapChainImage = true;
		swapColor.swapChainImageIndex = ctx.swapChainImageIndex;
		swapColor.loadOp = LoadOp::DontCare;
		swapColor.storeOp = StoreOp::Store;

		RenderPassDescriptor passDesc{};
		passDesc.colorAttachments = {swapColor};
		passDesc.width = displayW;
		passDesc.height = displayH;

		const PipelineResourceCache* toneMapPipeline = resourceManager->GetPipelineData("PostProcess::ToneMap");
		const std::array<Handle, 2>	 toneMapSets = {ctx.globalDescriptorSet, toneMapSet};

		renderingDevice->BeginRenderPass(passDesc);
		renderingDevice->DrawFullscreen(toneMapPipeline->pipeline, toneMapPipeline->layoutName, toneMapSets);
		renderingDevice->EndRenderPass();
	}
}
