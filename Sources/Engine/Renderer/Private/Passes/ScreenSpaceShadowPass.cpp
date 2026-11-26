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

#include <Common/RenderSettings.hpp>
#include <Math/Vector4.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>
#include <algorithm>
#include <cmath>

namespace BetterThanNothing
{
	namespace
	{
		constexpr int32_t  kBendWaveSize = 64;
		constexpr uint32_t kBendFlagIgnoreEdgePixels = 1u << 0;
		constexpr uint32_t kBendFlagUsePrecisionOffset = 1u << 1;
		constexpr uint32_t kBendFlagBilinearOffsetMode = 1u << 2;
		constexpr uint32_t kBendFlagUseEarlyOut = 1u << 3;
		constexpr uint32_t kBendFlagDebugEdgeMask = 1u << 4;
		constexpr uint32_t kBendFlagDebugThreadIndex = 1u << 5;
		constexpr uint32_t kBendFlagDebugWaveIndex = 1u << 6;

		int32_t FloorDiv(const int32_t value, const int32_t divisor)
		{
			return static_cast<int32_t>(std::floor(static_cast<double>(value) / static_cast<double>(divisor)));
		}

		int32_t CeilDiv(const int32_t value, const int32_t divisor)
		{
			return static_cast<int32_t>(std::ceil(static_cast<double>(value) / static_cast<double>(divisor)));
		}
	}

	SSSDispatchList ScreenSpaceShadowPass::BuildDispatchList(
		const Matrix4& unjitteredViewProj,
		const Vector3& sunDirection,
		const uint32_t viewportWidth,
		const uint32_t viewportHeight)
	{
		SSSDispatchList list{};
		if (viewportWidth == 0 || viewportHeight == 0)
			return list;

		const Vector4 lightProj = unjitteredViewProj * Vector4(sunDirection.GetX(), sunDirection.GetY(), sunDirection.GetZ(), 0.0f);

		const float rawW = lightProj.GetW();
		const bool	inFront = rawW >= 0.0f;
		const float safeW = inFront ? std::max(rawW, 0.003f) : std::min(rawW, -0.003f);

		const float ndcX = lightProj.GetX() / safeW;
		const float ndcY = lightProj.GetY() / safeW;
		const float lightZ = (std::abs(rawW) > 1e-5f) ? (lightProj.GetZ() / rawW) : 1.0f;

		const float widthF = static_cast<float>(viewportWidth);
		const float heightF = static_cast<float>(viewportHeight);

		float lightCoordX = (ndcX * 0.5f + 0.5f) * widthF;
		float lightCoordY = (ndcY * 0.5f + 0.5f) * heightF;

		constexpr float kMaxLightCoordRadius = 250000.0f;
		const float		centerX = widthF * 0.5f;
		const float		centerY = heightF * 0.5f;
		const float		dx = lightCoordX - centerX;
		const float		dy = lightCoordY - centerY;
		const float		distSq = dx * dx + dy * dy;
		if (distSq > kMaxLightCoordRadius * kMaxLightCoordRadius)
		{
			const float scale = kMaxLightCoordRadius / std::sqrt(distSq);
			lightCoordX = centerX + dx * scale;
			lightCoordY = centerY + dy * scale;
		}

		list.lightCoordinate[0] = lightCoordX;
		list.lightCoordinate[1] = lightCoordY;
		list.lightCoordinate[2] = lightZ;
		list.lightCoordinate[3] = inFront ? 1.0f : -1.0f;

		const int32_t lightPixelX = static_cast<int32_t>(std::floor(lightCoordX));
		const int32_t lightPixelY = static_cast<int32_t>(std::floor(lightCoordY));

		const int32_t relMinX = -lightPixelX;
		const int32_t relMaxX = static_cast<int32_t>(viewportWidth) - lightPixelX;
		const int32_t relMinY = -lightPixelY;
		const int32_t relMaxY = static_cast<int32_t>(viewportHeight) - lightPixelY;

		const int32_t minGX = FloorDiv(relMinX, kBendWaveSize) - 1;
		const int32_t maxGX = CeilDiv(relMaxX, kBendWaveSize) + 1;
		const int32_t minGY = FloorDiv(relMinY, kBendWaveSize) - 1;
		const int32_t maxGY = CeilDiv(relMaxY, kBendWaveSize) + 1;

		auto emitDispatch = [&](const int32_t gx0, const int32_t gx1, const int32_t gy0, const int32_t gy1) {
			if (gx0 > gx1 || gy0 > gy1 || list.dispatchCount >= list.dispatches.size())
				return;

			SSSDispatchItem& d = list.dispatches[list.dispatchCount++];
			d.waveOffsetX = gx0 * kBendWaveSize;
			d.waveOffsetY = gy0 * kBendWaveSize;
			d.waveCountX = static_cast<uint32_t>(kBendWaveSize);
			d.waveCountY = static_cast<uint32_t>(gx1 - gx0 + 1);
			d.waveCountZ = static_cast<uint32_t>(gy1 - gy0 + 1);
		};

		const bool containsOrigin = (minGX <= 0 && maxGX >= 0 && minGY <= 0 && maxGY >= 0);
		if (!containsOrigin)
		{
			emitDispatch(minGX, maxGX, minGY, maxGY);
		}
		else
		{
			if (minGY < 0)
				emitDispatch(minGX, maxGX, minGY, -1);
			if (maxGY > 0)
				emitDispatch(minGX, maxGX, 1, maxGY);
			if (minGX < 0)
				emitDispatch(minGX, -1, 0, 0);
			if (maxGX > 0)
				emitDispatch(1, maxGX, 0, 0);
		}

		return list;
	}

	void ScreenSpaceShadowPass::Execute(RenderPassContext& ctx)
	{
		const auto& settings = RenderSettings::Get();
		if (!settings.enableScreenSpaceShadows || !ctx.renderingDevice || !ctx.sssTarget.IsValid() || !ctx.sssDescriptorSet.IsValid())
			return;
		if (ctx.viewportWidth == 0 || ctx.viewportHeight == 0)
			return;

		const Vector3		  sunDir = -ctx.lightDirection;
		const SSSDispatchList list = BuildDispatchList(ctx.unjitteredViewProj, sunDir, ctx.viewportWidth, ctx.viewportHeight);
		if (list.dispatchCount == 0)
			return;

		RenderingDevice* renderingDevice = ctx.renderingDevice;
		renderingDevice->TransitionTexture(ctx.depthTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.sssTarget, ImageLayout::General);

		const PipelineResourceCache* pipeline = ctx.resourceManager->GetPipelineData("SSS::Bend");
		renderingDevice->BindComputePipeline(pipeline->pipeline);

		const std::array<Handle, 1> sets = {ctx.sssDescriptorSet};
		renderingDevice->BindComputeDescriptorSets("SSS::Bend", 0, sets);

		uint32_t flags = kBendFlagUsePrecisionOffset;
		if (settings.sssIgnoreEdgePixels)
			flags |= kBendFlagIgnoreEdgePixels;
		if (settings.sssBilinearOffsetMode)
			flags |= kBendFlagBilinearOffsetMode;
		if (settings.sssUseEarlyOut)
			flags |= kBendFlagUseEarlyOut;
		if (settings.sssDebugMode == 1)
			flags |= kBendFlagDebugEdgeMask;
		else if (settings.sssDebugMode == 2)
			flags |= kBendFlagDebugThreadIndex;
		else if (settings.sssDebugMode == 3)
			flags |= kBendFlagDebugWaveIndex;

		BendSSSPushConstants pc{};
		pc.lightCoordinate[0] = list.lightCoordinate[0];
		pc.lightCoordinate[1] = list.lightCoordinate[1];
		pc.lightCoordinate[2] = list.lightCoordinate[2];
		pc.lightCoordinate[3] = list.lightCoordinate[3];
		pc.invDepthTextureSize[0] = 1.0f / static_cast<float>(ctx.viewportWidth);
		pc.invDepthTextureSize[1] = 1.0f / static_cast<float>(ctx.viewportHeight);
		pc.surfaceThickness = settings.sssSurfaceThickness;
		pc.bilinearThreshold = settings.sssBilinearThreshold;
		pc.shadowContrast = settings.sssShadowContrast;
		pc.farDepthValue = 1.0f;
		pc.nearDepthValue = 0.0f;
		pc.flags = flags;
		pc.depthBounds[0] = 0.0f;
		pc.depthBounds[1] = 1.0f;
		pc.viewportWidth = static_cast<int32_t>(ctx.viewportWidth);
		pc.viewportHeight = static_cast<int32_t>(ctx.viewportHeight);

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
