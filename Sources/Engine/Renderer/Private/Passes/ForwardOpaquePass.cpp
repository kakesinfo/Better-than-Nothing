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

#include "ForwardOpaquePass.hpp"

#include <RenderingDevice/RenderPassDescription.hpp>
#include <RenderingDevice/RenderingDevice.hpp>

namespace BetterThanNothing
{
	void ForwardOpaquePass::Execute(RenderPassContext& ctx)
	{
		RenderingDevice* renderingDevice = ctx.renderingDevice;
		const bool		 hasTransparent = ctx.transparentStream && !ctx.transparentStream->Empty();

		ColorAttachmentDesc colorAttachment{};
		colorAttachment.loadOp = LoadOp::Clear;
		colorAttachment.clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
		if (ctx.useMsaa)
		{
			colorAttachment.target = ctx.colorTarget;
			colorAttachment.storeOp = hasTransparent ? StoreOp::Store : StoreOp::DontCare;
			colorAttachment.resolveToSwapChain = !hasTransparent;
			colorAttachment.swapChainImageIndex = ctx.swapChainImageIndex;
		}
		else
		{
			colorAttachment.useSwapChainImage = true;
			colorAttachment.swapChainImageIndex = ctx.swapChainImageIndex;
			colorAttachment.storeOp = StoreOp::Store;
		}

		DepthAttachmentDesc depthAttachment{};
		depthAttachment.target = ctx.depthTarget;
		depthAttachment.loadOp = LoadOp::Clear;
		depthAttachment.storeOp = hasTransparent ? StoreOp::Store : StoreOp::DontCare;
		depthAttachment.clearDepth = 1.0f;
		depthAttachment.clearStencil = 0;

		RenderPassDescriptor passDesc{};
		passDesc.colorAttachments = {colorAttachment};
		passDesc.depthAttachment = depthAttachment;
		passDesc.width = ctx.viewportWidth;
		passDesc.height = ctx.viewportHeight;

		const DrawStreamView drawView = ctx.opaqueStream->Build();

		renderingDevice->BeginRenderPass(passDesc);
		renderingDevice->ExecuteDrawStream(drawView);
		renderingDevice->EndRenderPass();
	}
}
