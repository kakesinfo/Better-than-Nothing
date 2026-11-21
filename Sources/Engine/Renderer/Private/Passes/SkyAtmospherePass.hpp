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

#include <Renderer/RenderPass.hpp>

namespace BetterThanNothing
{
	class SkyAtmospherePass final : public IRenderPass
	{
	public:
		static constexpr uint32_t kTransmittanceWidth = 256;
		static constexpr uint32_t kTransmittanceHeight = 64;
		static constexpr uint32_t kMultiScatteringSize = 32;
		static constexpr uint32_t kSkyViewWidth = 192;
		static constexpr uint32_t kSkyViewHeight = 108;

		// Irradiance / Split-Sum IBL LUT resolutions
		static constexpr uint32_t kBrdfLutSize = 256;
		static constexpr uint32_t kIrradianceWidth = 64;
		static constexpr uint32_t kIrradianceHeight = 32;
		static constexpr uint32_t kPrefilterWidth = 128;
		static constexpr uint32_t kPrefilterHeight = 320; // 5 roughness bands x 64

		[[nodiscard]] std::string_view GetName() const override { return "SkyAtmospherePass"; }
		void						   Execute(RenderPassContext& ctx) override;
	};
}
