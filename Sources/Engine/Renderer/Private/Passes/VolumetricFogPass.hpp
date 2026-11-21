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
#include <cstdint>

namespace BetterThanNothing
{
	struct VolumetricFogPushConstants
	{
		float	 density = 0.011f;
		float	 heightFalloff = 0.12f;
		float	 baseHeight = -1.5f;
		float	 maxDistance = 160.0f;

		float	 anisotropy = 0.65f;
		float	 noiseStrength = 0.55f;
		uint32_t stepCount = 32;
		uint32_t flags = 0;
	};

	static_assert(sizeof(VolumetricFogPushConstants) == 32);

	class VolumetricFogPass final : public IRenderPass
	{
	public:
		[[nodiscard]] std::string_view GetName() const override { return "VolumetricFogPass"; }
		void						   Execute(RenderPassContext& ctx) override;
	};
}
