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

#include <Math/Vector3.hpp>
#include <Renderer/RenderPass.hpp>
#include <cstdint>

namespace BetterThanNothing
{
	struct SkyViewPushConstants
	{
		float skyLuminanceNits = 8000.0f;
		float pad0 = 0.0f;
		float pad1 = 0.0f;
		float pad2 = 0.0f;
	};

	struct SkyRenderPushConstants
	{
		float	 skyLuminanceNits = 8000.0f;
		float	 cloudCoverage = 0.45f;
		float	 cloudSpeed = 8.0f;
		uint32_t enableClouds = 1u;
	};

	struct SkyLutDirtyState
	{
		bool	brdfLutDirty = true;
		bool	atmosphereStaticLutsDirty = true;
		bool	skyIblLutsDirty = true;
		Vector3 lastLightDirection{0.0f, 0.0f, 0.0f};
		float	lastSkyLuminanceNits = -1.0f;
		float	lastAltitudeKm = -1.0f;
		float	lastMieAnisotropy = -1.0f;
		float	lastMultiScatteringFactor = -1.0f;
		float	lastDensityScale = -1.0f;
	};

	class SkyAtmospherePass final : public IRenderPass
	{
	public:
		[[nodiscard]] std::string_view GetName() const override { return "SkyAtmospherePass"; }
		void						   Execute(RenderPassContext& ctx) override;

		static void EvaluateDirtyFlags(
			SkyLutDirtyState& state,
			const Vector3&	  lightDirection,
			float			  skyLuminanceNits,
			float			  altitudeKm = 0.2f,
			float			  mieAnisotropy = 0.8f,
			float			  multiScatteringFactor = 1.0f,
			float			  densityScale = 1.0f
		);

	private:
		SkyLutDirtyState m_dirtyState{};
	};
}
