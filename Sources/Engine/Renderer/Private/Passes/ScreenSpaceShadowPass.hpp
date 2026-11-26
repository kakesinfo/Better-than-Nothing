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

#pragma once

#include <Math/Vector2.hpp>
#include <Math/Vector4.hpp>
#include <Renderer/RenderPass.hpp>
#include <array>
#include <cstdint>

namespace BetterThanNothing
{
	struct BendSSSPushConstants
	{
		Vector4 lightCoordinate{0.0f, 0.0f, 0.0f, 1.0f};
		int32_t waveOffset[2]{0, 0};
		float	invDepthTextureSize[2]{0.0f, 0.0f};
		float	surfaceThickness = 0.005f;
		float	bilinearThreshold = 0.02f;
		float	shadowContrast = 1.0f;
		float	nearDepthValue = 0.0f;
		float	farDepthValue = 1.0f;
		int32_t debugMode = 0;
		int32_t pad0 = 0;
		int32_t pad1 = 0;
	};

	struct SSSDispatchItem
	{
		uint32_t waveCountX = 0;
		uint32_t waveCountY = 0;
		uint32_t waveCountZ = 1;
		int32_t	 waveOffsetX = 0;
		int32_t	 waveOffsetY = 0;
	};

	struct SSSDispatchList
	{
		Vector4						   lightCoordinate{0.0f, 0.0f, 0.0f, 1.0f};
		std::array<SSSDispatchItem, 4> dispatches{};
		uint32_t					   dispatchCount = 0;
	};

	class ScreenSpaceShadowPass final : public IRenderPass
	{
	public:
		[[nodiscard]] std::string_view GetName() const override { return "ScreenSpaceShadowPass"; }
		void						   Execute(RenderPassContext& ctx) override;

		[[nodiscard]] static SSSDispatchList BuildDispatchList(const Vector4& lightProjection, int32_t viewportWidth, int32_t viewportHeight, int32_t waveSize = 64);
	};
}
