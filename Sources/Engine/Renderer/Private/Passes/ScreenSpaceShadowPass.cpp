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

#include "ScreenSpaceShadowPass.hpp"

#include <Common/RenderSettings.hpp>
#include <Math/Vector4.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>
#include <algorithm>
#include <array>
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

	ScreenSpaceShadowPass::DispatchList ScreenSpaceShadowPass::BuildDispatchList(
		const Matrix4& unjitteredViewProj,
		const Vector3& sunDirection,
		const uint32_t viewportWidth,
		const uint32_t viewportHeight
	)
	{
		DispatchList list{};
		if (viewportWidth == 0 || viewportHeight == 0)
			return list;

		// Project directional sun vector (pointing toward the sun, w = 0) into Vulkan clip space (Y-down, Z in [0, 1])
		const Vector4 lightProj = unjitteredViewProj * Vector4(sunDirection.GetX(), sunDirection.GetY(), sunDirection.GetZ(), 0.0f);

		const float rawW = lightProj.GetW();
		const bool	inFront = rawW >= 0.0f;
		const float safeW = inFront ? std::max(rawW, 0.003f) : std::min(rawW, -0.003f);

		const float ndcX = lightProj.GetX() / safeW;
		const float ndcY = lightProj.GetY() / safeW;
		// For a directional light at infinity in standard-Z [0, 1], clip.z / clip.w == far / (far - near) ~= 1.0
		const float lightZ = (std::abs(rawW) > 1e-5f) ? (lightProj.GetZ() / rawW) : 1.0f;

		const float widthF = static_cast<float>(viewportWidth);
		const float heightF = static_cast<float>(viewportHeight);

		// Perspective projection already flips Y for Vulkan, so NDC [-1, 1] maps directly to [0, viewport]
		float lightCoordX = (ndcX * 0.5f + 0.5f) * widthF;
		float lightCoordY = (ndcY * 0.5f + 0.5f) * heightF;

		// Keep coordinates within float32 subpixel precision even when sun is nearly perpendicular to view
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

		// Expand by 1 wavefront ring cell on each side to guarantee 100% coverage of diagonal ring wedges
		const int32_t minGX = FloorDiv(relMinX, kBendWaveSize) - 1;
		const int32_t maxGX = CeilDiv(relMaxX, kBendWaveSize) + 1;
		const int32_t minGY = FloorDiv(relMinY, kBendWaveSize) - 1;
		const int32_t maxGY = CeilDiv(relMaxY, kBendWaveSize) + 1;

		auto emitDispatch = [&](const int32_t gx0, const int32_t gx1, const int32_t gy0, const int32_t gy1) {
			if (gx0 > gx1 || gy0 > gy1 || list.dispatchCount >= list.dispatches.size())
				return;

			DispatchData& d = list.dispatches[list.dispatchCount++];
			d.waveOffset[0] = gx0 * kBendWaveSize;
			d.waveOffset[1] = gy0 * kBendWaveSize;
			d.waveCount[0] = static_cast<uint32_t>(kBendWaveSize);
			d.waveCount[1] = static_cast<uint32_t>(gx1 - gx0 + 1);
			d.waveCount[2] = static_cast<uint32_t>(gy1 - gy0 + 1);
		};

		// Cell (0, 0) is degenerate in ComputeWavefrontExtents (sign(0, 0) == 0); split into up to 4 sub-rects if contained
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
		if (!ctx.bendSssActive || !ctx.bendSssDescriptorSet.IsValid() || ctx.viewportWidth == 0 || ctx.viewportHeight == 0)
			return;

		RenderingDevice* renderingDevice = ctx.renderingDevice;
		ResourceManager* resourceManager = ctx.resourceManager;
		const auto&		 settings = RenderSettings::Get();

		renderingDevice->TransitionTexture(ctx.depthTarget, ImageLayout::ShaderReadOnly);
		renderingDevice->TransitionTexture(ctx.screenSpaceShadow, ImageLayout::General);

		const DispatchList dispatchList = BuildDispatchList(
			ctx.unjitteredViewProj,
			ctx.sunDirection,
			ctx.viewportWidth,
			ctx.viewportHeight
		);

		if (dispatchList.dispatchCount == 0)
			return;

		const PipelineResourceCache* sssPipeline = resourceManager->GetPipelineData("Bend::SSS");
		const std::array<Handle, 1>	 sets = {ctx.bendSssDescriptorSet};

		renderingDevice->BindComputePipeline(sssPipeline->pipeline);
		renderingDevice->BindComputeDescriptorSets(sssPipeline->layoutName, 0, sets);

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

		BendSssPushConstants push{};
		push.lightCoordinate[0] = dispatchList.lightCoordinate[0];
		push.lightCoordinate[1] = dispatchList.lightCoordinate[1];
		push.lightCoordinate[2] = dispatchList.lightCoordinate[2];
		push.lightCoordinate[3] = dispatchList.lightCoordinate[3];
		push.invDepthTextureSize[0] = 1.0f / static_cast<float>(ctx.viewportWidth);
		push.invDepthTextureSize[1] = 1.0f / static_cast<float>(ctx.viewportHeight);
		push.surfaceThickness = settings.sssSurfaceThickness;
		push.bilinearThreshold = settings.sssBilinearThreshold;
		push.shadowContrast = settings.sssShadowContrast;
		push.farDepthValue = 1.0f;
		push.nearDepthValue = 0.0f;
		push.flags = flags;
		push.depthBounds[0] = 0.0f;
		push.depthBounds[1] = 1.0f;
		push.viewportWidth = static_cast<int32_t>(ctx.viewportWidth);
		push.viewportHeight = static_cast<int32_t>(ctx.viewportHeight);

		for (uint32_t i = 0; i < dispatchList.dispatchCount; ++i)
		{
			const DispatchData& d = dispatchList.dispatches[i];
			push.waveOffset[0] = d.waveOffset[0];
			push.waveOffset[1] = d.waveOffset[1];
			renderingDevice->PushComputeConstants(sssPipeline->layoutName, &push, sizeof(push));
			renderingDevice->Dispatch(d.waveCount[0], d.waveCount[1], d.waveCount[2]);
		}
	}
}
