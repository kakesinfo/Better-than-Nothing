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

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace BetterThanNothing::TerrainNoise
{
	inline uint32_t Hash2D(int32_t x, int32_t y, uint32_t seed) noexcept
	{
		uint32_t h = static_cast<uint32_t>(x) * 374761393u + static_cast<uint32_t>(y) * 668265263u + seed * 1442695041u;
		h = (h ^ (h >> 13u)) * 1274126177u;
		return h ^ (h >> 16u);
	}

	inline float Quintic(const float t) noexcept
	{
		return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
	}

	inline float Smoothstep(const float edge0, const float edge1, const float x) noexcept
	{
		const float t = std::clamp((x - edge0) / std::max(edge1 - edge0, 1e-6f), 0.0f, 1.0f);
		return t * t * (3.0f - 2.0f * t);
	}

	inline float QuinticSmoothstep(const float edge0, const float edge1, const float x) noexcept
	{
		const float t = std::clamp((x - edge0) / std::max(edge1 - edge0, 1e-6f), 0.0f, 1.0f);
		return Quintic(t);
	}

	// 2D Gradient (Perlin) noise returning values in approximately [-1, 1]
	inline float GradientNoise2D(const float x, const float y, const uint32_t seed) noexcept
	{
		const int32_t xi = static_cast<int32_t>(std::floor(x));
		const int32_t yi = static_cast<int32_t>(std::floor(y));
		const float	  xf = x - static_cast<float>(xi);
		const float	  yf = y - static_cast<float>(yi);

		const float u = Quintic(xf);
		const float v = Quintic(yf);

		auto gradDot = [seed](const int32_t ix, const int32_t iy, const float dx, const float dy) noexcept -> float {
			const uint32_t h = Hash2D(ix, iy, seed) & 7u;
			static constexpr float gx[8] = {1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f};
			static constexpr float gy[8] = {1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f};
			return gx[h] * dx + gy[h] * dy;
		};

		const float n00 = gradDot(xi, yi, xf, yf);
		const float n10 = gradDot(xi + 1, yi, xf - 1.0f, yf);
		const float n01 = gradDot(xi, yi + 1, xf, yf - 1.0f);
		const float n11 = gradDot(xi + 1, yi + 1, xf - 1.0f, yf - 1.0f);

		const float nx0 = n00 + (n10 - n00) * u;
		const float nx1 = n01 + (n11 - n01) * u;
		return nx0 + (nx1 - nx0) * v;
	}

	inline float FBM2D(float x, float y, const uint32_t seed, const int octaves = 5, const float lacunarity = 2.02f, const float gain = 0.5f) noexcept
	{
		float sum = 0.0f;
		float amp = 0.5f;
		float norm = 0.0f;
		float freq = 1.0f;
		for (int i = 0; i < octaves; ++i)
		{
			sum += amp * GradientNoise2D(x * freq, y * freq, seed + static_cast<uint32_t>(i) * 1013u);
			norm += amp;
			freq *= lacunarity;
			amp *= gain;
		}
		return norm > 0.0f ? (sum / norm) : 0.0f;
	}

	inline float RidgedFBM2D(float x, float y, const uint32_t seed, const int octaves = 4, const float lacunarity = 2.05f, const float gain = 0.5f) noexcept
	{
		float sum = 0.0f;
		float amp = 0.5f;
		float norm = 0.0f;
		float freq = 1.0f;
		for (int i = 0; i < octaves; ++i)
		{
			const float n = 1.0f - std::abs(GradientNoise2D(x * freq, y * freq, seed + static_cast<uint32_t>(i) * 2029u));
			sum += amp * (n * n);
			norm += amp;
			freq *= lacunarity;
			amp *= gain;
		}
		return norm > 0.0f ? (sum / norm) : 0.0f;
	}

	class SeededRng
	{
	public:
		explicit SeededRng(const uint32_t seed) : m_state(seed ? seed : 0x9e3779b9u) {}

		uint32_t NextU32() noexcept
		{
			uint32_t x = m_state;
			x ^= x << 13u;
			x ^= x >> 17u;
			x ^= x << 5u;
			m_state = x;
			return x;
		}

		float NextFloat() noexcept
		{
			return static_cast<float>(NextU32() & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
		}

		float Range(const float minVal, const float maxVal) noexcept
		{
			return minVal + (maxVal - minVal) * NextFloat();
		}

	private:
		uint32_t m_state = 1u;
	};
}
