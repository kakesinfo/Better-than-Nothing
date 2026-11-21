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

#include "ForwardTransparentPass.hpp"

#include <RenderingDevice/RenderPassDescription.hpp>
#include <RenderingDevice/RenderingDevice.hpp>

namespace BetterThanNothing
{
	void ForwardTransparentPass::Execute(RenderPassContext& ctx)
	{
		if (!ctx.transparentStream || ctx.transparentStream->Empty())
			return;

		RenderingDevice* device = ctx.renderingDevice;

		ColorAttachmentDesc colorAttachment{};
		colorAttachment.loadOp = LoadOp::Load;
		if (ctx.useMsaa)
		{
			colorAttachment.target = ctx.colorTarget;
			colorAttachment.storeOp = StoreOp::DontCare;
			colorAttachment.resolveToSwapChain = true;
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
		depthAttachment.loadOp = LoadOp::Load;
		depthAttachment.storeOp = StoreOp::DontCare;

		RenderPassDescriptor passDesc{};
		passDesc.colorAttachments = {colorAttachment};
		passDesc.depthAttachment = depthAttachment;
		passDesc.width = ctx.viewportWidth;
		passDesc.height = ctx.viewportHeight;

		const DrawStreamView drawView = ctx.transparentStream->Build();

		device->BeginRenderPass(passDesc);
		device->ExecuteDrawStream(drawView);
		device->EndRenderPass();
	}
}
