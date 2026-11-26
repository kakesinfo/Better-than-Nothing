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

#include <Renderer/RenderPass.hpp>
#include <array>
#include <cstdint>
#include <utility>

namespace BetterThanNothing
{
	struct RTShadowPushConstants
	{
		float	 sunAngularRadius = 0.0093f;
		uint32_t frameIndex = 0;
		uint32_t enableRTShadows = 1;
		uint32_t pad = 0;
	};

	class RTShadowPass final : public IRenderPass
	{
	public:
		[[nodiscard]] std::string_view GetName() const override { return "RTShadowPass"; }
		void						   Execute(RenderPassContext& ctx) override;

		[[nodiscard]] static std::pair<uint32_t, uint32_t> ComputeDispatchGroups(uint32_t width, uint32_t height);
		[[nodiscard]] static constexpr std::array<int32_t, 3> GetFilterStepSizes() { return {1, 2, 4}; }
	};
}
