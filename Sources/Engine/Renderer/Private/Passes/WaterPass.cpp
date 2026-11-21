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

#include "WaterPass.hpp"

#include <RenderingDevice/RenderPassDescription.hpp>
#include <RenderingDevice/RenderingDevice.hpp>

namespace BetterThanNothing
{
	void WaterPass::Execute(RenderPassContext& ctx)
	{
		if (!ctx.waterStream || ctx.waterStream->Empty())
			return;

		RenderingDevice* renderingDevice = ctx.renderingDevice;

		// Snapshot opaque HDR color & depth for screen-space refraction and Beer-Lambert depth absorption
		renderingDevice->CopyTexture(ctx.hdrColor, ctx.waterSceneColorCopy);
		renderingDevice->CopyTexture(ctx.depthTarget, ctx.waterDepthCopy);

		renderingDevice->TransitionTexture(ctx.waterSceneColorCopy, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.waterDepthCopy, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.transmittanceLUT, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.irradianceLUT, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.prefilteredEnvLUT, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.brdfLUT, ImageLayout::ShaderReadOnly);

		ColorAttachmentDesc colorAttachment{};
		colorAttachment.target = ctx.hdrColor;
		colorAttachment.loadOp = LoadOp::Load;
		colorAttachment.storeOp = StoreOp::Store;

		DepthAttachmentDesc depthAttachment{};
		depthAttachment.target = ctx.depthTarget;
		depthAttachment.loadOp = LoadOp::Load;
		depthAttachment.storeOp = StoreOp::Store;

		RenderPassDescriptor passDesc{};
		passDesc.colorAttachments = {colorAttachment};
		passDesc.depthAttachment = depthAttachment;
		passDesc.width = ctx.viewportWidth;
		passDesc.height = ctx.viewportHeight;

		const DrawStreamView drawView = ctx.waterStream->Build();

		renderingDevice->BeginRenderPass(passDesc);
		renderingDevice->ExecuteDrawStream(drawView);
		renderingDevice->EndRenderPass();
	}
}
