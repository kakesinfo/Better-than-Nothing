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

#pragma once

#include "Renderer/RenderPass.hpp"
#include <array>
#include <cstdint>

namespace BetterThanNothing
{
	struct BendSssPushConstants
	{
		float	 lightCoordinate[4]{0.0f, 0.0f, 1.0f, 1.0f};
		int32_t	 waveOffset[2]{0, 0};
		float	 invDepthTextureSize[2]{0.0f, 0.0f};

		float	 surfaceThickness = 0.005f;
		float	 bilinearThreshold = 0.02f;
		float	 shadowContrast = 4.0f;
		float	 farDepthValue = 1.0f;

		float	 nearDepthValue = 0.0f;
		uint32_t flags = 0;
		float	 depthBounds[2]{0.0f, 1.0f};

		int32_t	 viewportWidth = 0;
		int32_t	 viewportHeight = 0;
		int32_t	 pad0 = 0;
		int32_t	 pad1 = 0;
	};

	static_assert(sizeof(BendSssPushConstants) == 80);

	class ScreenSpaceShadowPass final : public IRenderPass
	{
	public:
		struct DispatchData
		{
			uint32_t waveCount[3]{0, 0, 0};
			int32_t	 waveOffset[2]{0, 0};
		};

		struct DispatchList
		{
			float						 lightCoordinate[4]{0.0f, 0.0f, 1.0f, 1.0f};
			std::array<DispatchData, 4>	 dispatches{};
			uint32_t					 dispatchCount = 0;
		};

		[[nodiscard]] std::string_view GetName() const override { return "ScreenSpaceShadowPass"; }
		void						   Execute(RenderPassContext& ctx) override;

		static DispatchList BuildDispatchList(
			const Matrix4& unjitteredViewProj,
			const Vector3& sunDirection,
			uint32_t	   viewportWidth,
			uint32_t	   viewportHeight
		);
	};
}
