/*
 *	Better than Nothing
 *	Copyright (C) 2025 kakesinfo
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

#include "Types.hpp"

namespace BetterThanNothing
{
	struct SamplerDescription
	{
		SamplerFilter	   magFilter = SamplerFilter::Linear;
		SamplerFilter	   minFilter = SamplerFilter::Linear;
		SamplerMipmapMode  mipmapMode = SamplerMipmapMode::Linear;
		SamplerAddressMode addressModeU = SamplerAddressMode::Repeat;
		SamplerAddressMode addressModeV = SamplerAddressMode::Repeat;
		SamplerAddressMode addressModeW = SamplerAddressMode::Repeat;
		bool			   anisotropyEnable = true;
		float			   maxAnisotropy = -1.0f; // If -1.0f, uses device maxAnisotropy
		float			   minLod = 0.0f;
		float			   maxLod = 1000.f; // If VK_LOD_CLAMP_NONE (1000.f), uses device maxLod
		float			   mipLodBias = 0.0f;
		bool			   compareEnable = false;
		CompareOp		   compareOp = CompareOp::Always;
		BorderColor		   borderColor = BorderColor::IntOpaqueBlack;
	};

	// clang-format off
	struct SamplerDescriptionHash
	{
		size_t operator()(SamplerDescription const& a) const noexcept
		{
			auto hash_combine = [](size_t& hash, const size_t value) {
				hash ^= value + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			};

			size_t hash = 0;
			hash_combine(hash, std::hash<size_t>{}(static_cast<size_t>(a.magFilter)));
			hash_combine(hash, std::hash<size_t>{}(static_cast<size_t>(a.minFilter)));
			hash_combine(hash, std::hash<size_t>{}(static_cast<size_t>(a.mipmapMode)));
			hash_combine(hash, std::hash<size_t>{}(static_cast<size_t>(a.addressModeU)));
			hash_combine(hash, std::hash<size_t>{}(static_cast<size_t>(a.addressModeV)));
			hash_combine(hash, std::hash<size_t>{}(static_cast<size_t>(a.addressModeW)));
			hash_combine(hash, std::hash<bool>{}(a.anisotropyEnable));

			// TODO: ensure float comp is safe
			hash_combine(hash, std::hash<float>{}(a.maxAnisotropy));
			hash_combine(hash, std::hash<float>{}(a.minLod));
			hash_combine(hash, std::hash<float>{}(a.maxLod));
			hash_combine(hash, std::hash<float>{}(a.mipLodBias));

			hash_combine(hash, std::hash<bool>{}(a.compareEnable));
			hash_combine(hash, std::hash<size_t>{}(static_cast<size_t>(a.compareOp)));
			hash_combine(hash, std::hash<size_t>{}(static_cast<size_t>(a.borderColor)));
			return hash;
		}
	};

	struct SamplerDescriptionEqual
	{
		bool operator()(SamplerDescription const& a, SamplerDescription const& b) const noexcept
		{
			return std::tie(
				a.magFilter, a.minFilter, a.mipmapMode,
				a.addressModeU, a.addressModeV, a.addressModeW,
				a.anisotropyEnable, a.maxAnisotropy,
				a.minLod, a.maxLod, a.mipLodBias,
				a.compareEnable, a.compareOp, a.borderColor
			) == std::tie(
				b.magFilter, b.minFilter, b.mipmapMode,
				b.addressModeU, b.addressModeV, b.addressModeW,
				b.anisotropyEnable, b.maxAnisotropy,
				b.minLod, b.maxLod, b.mipLodBias,
				b.compareEnable, b.compareOp, b.borderColor
			);
		}
	};
	// clang-format on
}
