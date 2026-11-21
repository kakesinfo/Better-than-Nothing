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

#include "EditorPass.hpp"

#include <Editor/Editor.hpp>
#include <RenderingDevice/RenderPassDescription.hpp>
#include <RenderingDevice/RenderingDevice.hpp>

namespace BetterThanNothing
{
	void EditorPass::Execute(RenderPassContext& ctx)
	{
		RenderingDevice* device = ctx.renderingDevice;

#ifndef NDEBUG
		if (ctx.editor)
		{
			ctx.editor->RenderPanels(ctx.scene);

			device->TransitionSwapChainImage(ctx.swapChainImageIndex, ImageLayout::ColorAttachment, ImageLayout::ColorAttachment);

			ColorAttachmentDesc colorAttachment{};
			colorAttachment.useSwapChainImage = true;
			colorAttachment.swapChainImageIndex = ctx.swapChainImageIndex;
			colorAttachment.loadOp = LoadOp::Load;
			colorAttachment.storeOp = StoreOp::Store;

			RenderPassDescriptor passDesc{};
			passDesc.colorAttachments = {colorAttachment};
			passDesc.width = ctx.viewportWidth;
			passDesc.height = ctx.viewportHeight;

			device->BeginRenderPass(passDesc);
			ctx.editor->RenderDrawData(device->GetCurrentCommandBuffer());
			device->EndRenderPass();
		}
#endif

		device->TransitionSwapChainImage(ctx.swapChainImageIndex, ImageLayout::ColorAttachment, ImageLayout::Present);
	}
}
