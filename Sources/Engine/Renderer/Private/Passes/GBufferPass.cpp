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

#include "GBufferPass.hpp"

#include <RenderingDevice/RenderPassDescription.hpp>
#include <RenderingDevice/RenderingDevice.hpp>

namespace BetterThanNothing
{
	void GBufferPass::Execute(RenderPassContext& ctx)
	{
		RenderingDevice* renderingDevice = ctx.renderingDevice;

		ColorAttachmentDesc albedoAttachment{};
		albedoAttachment.target = ctx.gbufferAlbedo;
		albedoAttachment.loadOp = LoadOp::Clear;
		albedoAttachment.storeOp = StoreOp::Store;
		albedoAttachment.clearColor = {0.0f, 0.0f, 0.0f, 0.0f};

		ColorAttachmentDesc normalAttachment{};
		normalAttachment.target = ctx.gbufferNormal;
		normalAttachment.loadOp = LoadOp::Clear;
		normalAttachment.storeOp = StoreOp::Store;
		normalAttachment.clearColor = {0.0f, 0.0f, 0.0f, 0.0f};

		ColorAttachmentDesc materialAttachment{};
		materialAttachment.target = ctx.gbufferMaterial;
		materialAttachment.loadOp = LoadOp::Clear;
		materialAttachment.storeOp = StoreOp::Store;
		materialAttachment.clearColor = {0.0f, 0.5f, 1.0f, 0.0f};

		ColorAttachmentDesc motionAttachment{};
		motionAttachment.target = ctx.gbufferMotion;
		motionAttachment.loadOp = LoadOp::Clear;
		motionAttachment.storeOp = StoreOp::Store;
		motionAttachment.clearColor = {0.0f, 0.0f, 0.0f, 0.0f};

		DepthAttachmentDesc depthAttachment{};
		depthAttachment.target = ctx.depthTarget;
		depthAttachment.loadOp = LoadOp::Clear;
		depthAttachment.storeOp = StoreOp::Store;
		depthAttachment.clearDepth = 1.0f;
		depthAttachment.clearStencil = 0;

		RenderPassDescriptor passDesc{};
		passDesc.colorAttachments = {albedoAttachment, normalAttachment, materialAttachment, motionAttachment};
		passDesc.depthAttachment = depthAttachment;
		passDesc.width = ctx.viewportWidth;
		passDesc.height = ctx.viewportHeight;

		const DrawStreamView drawView = ctx.opaqueStream->Build();

		renderingDevice->BeginRenderPass(passDesc);
		renderingDevice->ExecuteDrawStream(drawView);
		renderingDevice->EndRenderPass();
	}
}
