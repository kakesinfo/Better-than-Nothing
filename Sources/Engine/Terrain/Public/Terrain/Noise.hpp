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

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace BetterThanNothing
{
	[[nodiscard]] inline uint32_t Hash2D(const int32_t x, const int32_t z, const uint32_t seed = 0u) noexcept
	{
		uint32_t h = static_cast<uint32_t>(x) * 374761393u + static_cast<uint32_t>(z) * 668265263u + seed * 1442695041u;
		h = (h ^ (h >> 13)) * 1274126177u;
		return h ^ (h >> 16);
	}

	[[nodiscard]] inline float QuinticSmoothstep(const float t) noexcept
	{
		const float c = std::clamp(t, 0.0f, 1.0f);
		return c * c * c * (c * (c * 6.0f - 15.0f) + 10.0f);
	}

	[[nodiscard]] inline float GradientNoise2D(const float x, const float z, const uint32_t seed = 0u) noexcept
	{
		const int32_t xi = static_cast<int32_t>(std::floor(x));
		const int32_t zi = static_cast<int32_t>(std::floor(z));
		const float	  xf = x - static_cast<float>(xi);
		const float	  zf = z - static_cast<float>(zi);

		const float u = QuinticSmoothstep(xf);
		const float v = QuinticSmoothstep(zf);

		auto gradDot = [&](const int32_t gx, const int32_t gz, const float dx, const float dz) noexcept -> float {
			const uint32_t h = Hash2D(gx, gz, seed) & 7u;
			static constexpr float kDirs[8][2] = {
				{1.0f, 0.0f}, {-1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, -1.0f},
				{0.70710678f, 0.70710678f}, {-0.70710678f, 0.70710678f},
				{0.70710678f, -0.70710678f}, {-0.70710678f, -0.70710678f},
			};
			return kDirs[h][0] * dx + kDirs[h][1] * dz;
		};

		const float n00 = gradDot(xi, zi, xf, zf);
		const float n10 = gradDot(xi + 1, zi, xf - 1.0f, zf);
		const float n01 = gradDot(xi, zi + 1, xf, zf - 1.0f);
		const float n11 = gradDot(xi + 1, zi + 1, xf - 1.0f, zf - 1.0f);

		const float nx0 = n00 + u * (n10 - n00);
		const float nx1 = n01 + u * (n11 - n01);
		return nx0 + v * (nx1 - nx0);
	}

	[[nodiscard]] inline float FBM2D(const float x, const float z, const int octaves = 4, const float lacunarity = 2.0f, const float gain = 0.5f, const uint32_t seed = 0u) noexcept
	{
		float sum = 0.0f;
		float amp = 0.5f;
		float freq = 1.0f;
		float norm = 0.0f;
		for (int i = 0; i < octaves; ++i)
		{
			sum += amp * GradientNoise2D(x * freq, z * freq, seed + static_cast<uint32_t>(i) * 1013u);
			norm += amp;
			freq *= lacunarity;
			amp *= gain;
		}
		return (norm > 0.0f) ? (sum / norm) : 0.0f;
	}

	[[nodiscard]] inline float RidgedFBM2D(const float x, const float z, const int octaves = 4, const float lacunarity = 2.0f, const float gain = 0.5f, const uint32_t seed = 0u) noexcept
	{
		float sum = 0.0f;
		float amp = 0.5f;
		float freq = 1.0f;
		float norm = 0.0f;
		for (int i = 0; i < octaves; ++i)
		{
			const float n = 1.0f - std::abs(GradientNoise2D(x * freq, z * freq, seed + static_cast<uint32_t>(i) * 1999u));
			sum += amp * (n * n);
			norm += amp;
			freq *= lacunarity;
			amp *= gain;
		}
		return (norm > 0.0f) ? (sum / norm) : 0.0f;
	}

	class SeededRng
	{
	public:
		explicit SeededRng(const uint32_t seed = 0x12345678u) noexcept
			: m_state(seed == 0u ? 0x6D2B79F5u : seed)
		{
		}

		[[nodiscard]] uint32_t NextU32() noexcept
		{
			uint32_t x = m_state;
			x ^= x << 13;
			x ^= x >> 17;
			x ^= x << 5;
			m_state = x;
			return x;
		}

		[[nodiscard]] float NextFloat() noexcept
		{
			return static_cast<float>(NextU32() >> 8) * (1.0f / 16777216.0f);
		}

		[[nodiscard]] float NextRange(const float minVal, const float maxVal) noexcept
		{
			return minVal + NextFloat() * (maxVal - minVal);
		}

	private:
		uint32_t m_state;
	};
}
