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
#ifndef NDEBUG
		if (!ctx.editor)
			return;

		RenderingDevice* renderingDevice = ctx.renderingDevice;

		ctx.editor->RenderPanels(ctx.scene);

		ColorAttachmentDesc colorAttachment{};
		colorAttachment.useSwapChainImage = true;
		colorAttachment.swapChainImageIndex = ctx.swapChainImageIndex;
		colorAttachment.loadOp = LoadOp::Load;
		colorAttachment.storeOp = StoreOp::Store;

		const uint32_t displayW = ctx.displayWidth > 0 ? ctx.displayWidth : ctx.viewportWidth;
		const uint32_t displayH = ctx.displayHeight > 0 ? ctx.displayHeight : ctx.viewportHeight;

		RenderPassDescriptor passDesc{};
		passDesc.colorAttachments = {colorAttachment};
		passDesc.width = displayW;
		passDesc.height = displayH;

		renderingDevice->BeginRenderPass(passDesc);
		ctx.editor->RenderDrawData();
		renderingDevice->EndRenderPass();
#else
		(void)ctx;
#endif
	}
}
