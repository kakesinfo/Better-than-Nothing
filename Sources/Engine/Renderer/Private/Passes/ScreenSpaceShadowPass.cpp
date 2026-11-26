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

#include "ScreenSpaceShadowPass.hpp"

#include <Renderer/RenderSettings.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>
#include <algorithm>
#include <cmath>

namespace BetterThanNothing
{
	SSSDispatchList ScreenSpaceShadowPass::BuildDispatchList(const Vector4& lightProjection, const int32_t viewportWidth, const int32_t viewportHeight, const int32_t waveSize)
	{
		SSSDispatchList result{};
		if (viewportWidth <= 0 || viewportHeight <= 0 || waveSize <= 0)
			return result;

		float lightPixelX = static_cast<float>(viewportWidth) * 0.5f;
		float lightPixelY = static_cast<float>(viewportHeight) * 0.5f;
		float lightDepth = 0.0f;

		if (std::abs(lightProjection.GetW()) > 1e-6f)
		{
			const float invW = 1.0f / lightProjection.GetW();
			const float ndcX = lightProjection.GetX() * invW;
			const float ndcY = lightProjection.GetY() * invW;
			lightDepth = lightProjection.GetZ() * invW;
			lightPixelX = (ndcX * 0.5f + 0.5f) * static_cast<float>(viewportWidth);
			lightPixelY = (ndcY * 0.5f + 0.5f) * static_cast<float>(viewportHeight);
		}

		result.lightCoordinate = Vector4(lightPixelX, lightPixelY, lightDepth, lightProjection.GetW());

		const int32_t lightXY[2] = {
			std::clamp(static_cast<int32_t>(std::floor(lightPixelX)), 0, viewportWidth),
			std::clamp(static_cast<int32_t>(std::floor(lightPixelY)), 0, viewportHeight),
		};

		const int32_t minBounds[2] = {0, 0};
		const int32_t maxBounds[2] = {viewportWidth, viewportHeight};

		auto addRegion = [&](const int32_t startX, const int32_t startY, const int32_t endX, const int32_t endY) {
			if (endX <= startX || endY <= startY || result.dispatchCount >= 4u)
				return;

			const uint32_t width = static_cast<uint32_t>(endX - startX);
			const uint32_t height = static_cast<uint32_t>(endY - startY);
			const uint32_t tilesX = (width + static_cast<uint32_t>(waveSize) - 1u) / static_cast<uint32_t>(waveSize);
			const uint32_t tilesY = (height + static_cast<uint32_t>(waveSize) - 1u) / static_cast<uint32_t>(waveSize);
			if (tilesX == 0 || tilesY == 0)
				return;

			SSSDispatchItem& item = result.dispatches[result.dispatchCount++];
			item.waveCountX = 1u;
			item.waveCountY = tilesX;
			item.waveCountZ = tilesY;
			item.waveOffsetX = startX;
			item.waveOffsetY = startY;
		};

		// Partition viewport into up to 4 quadrants around the projected light coordinate
		addRegion(minBounds[0], minBounds[1], lightXY[0], lightXY[1]);
		addRegion(lightXY[0], minBounds[1], maxBounds[0], lightXY[1]);
		addRegion(minBounds[0], lightXY[1], lightXY[0], maxBounds[1]);
		addRegion(lightXY[0], lightXY[1], maxBounds[0], maxBounds[1]);

		if (result.dispatchCount == 0)
			addRegion(minBounds[0], minBounds[1], maxBounds[0], maxBounds[1]);

		return result;
	}

	void ScreenSpaceShadowPass::Execute(RenderPassContext& ctx)
	{
		const auto& settings = RenderSettings::Get();
		if (!settings.enableScreenSpaceShadows || !ctx.renderingDevice || !ctx.sssTarget.IsValid() || !ctx.sssDescriptorSet.IsValid())
			return;

		const int32_t width = static_cast<int32_t>(ctx.viewportWidth);
		const int32_t height = static_cast<int32_t>(ctx.viewportHeight);
		if (width <= 0 || height <= 0)
			return;

		const Vector3		  negLightDir = -ctx.lightDirection;
		const Vector4		  lightProj = ctx.unjitteredViewProj * Vector4(negLightDir.GetX(), negLightDir.GetY(), negLightDir.GetZ(), 0.0f);
		const SSSDispatchList list = BuildDispatchList(lightProj, width, height, 64);
		if (list.dispatchCount == 0)
			return;

		RenderingDevice* renderingDevice = ctx.renderingDevice;
		renderingDevice->TransitionTexture(ctx.depthTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.sssTarget, ImageLayout::General);

		const PipelineResourceCache* pipeline = ctx.resourceManager->GetPipelineData("SSS::Bend");
		renderingDevice->BindComputePipeline(pipeline->pipeline);

		const std::array<Handle, 2> sets = {ctx.globalDescriptorSet, ctx.sssDescriptorSet};
		renderingDevice->BindComputeDescriptorSets("SSS::Bend", 0, sets);

		BendSSSPushConstants pc{};
		pc.lightCoordinate = list.lightCoordinate;
		pc.invDepthTextureSize[0] = 1.0f / static_cast<float>(width);
		pc.invDepthTextureSize[1] = 1.0f / static_cast<float>(height);
		pc.debugMode = settings.sssDebugMode;

		for (uint32_t i = 0; i < list.dispatchCount; ++i)
		{
			const SSSDispatchItem& item = list.dispatches[i];
			pc.waveOffset[0] = item.waveOffsetX;
			pc.waveOffset[1] = item.waveOffsetY;

			renderingDevice->PushComputeConstants("SSS::Bend", &pc, sizeof(pc));
			renderingDevice->Dispatch(item.waveCountX, item.waveCountY, item.waveCountZ);
		}

		renderingDevice->ComputeBarrier();
		renderingDevice->TransitionTexture(ctx.sssTarget, ImageLayout::ShaderReadOnly);
	}
}
