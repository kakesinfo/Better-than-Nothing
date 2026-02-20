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

#include <cstdint>

namespace BetterThanNothing
{
	enum class TextureFormat : std::uint8_t
	{
		Undefined = 0,
		R8G8B8A8_UNORM = 37,
		R8G8B8A8_SRGB = 43,
		D32_SFLOAT = 126,
		D24_UNORM_S8_UINT = 129,
		D32_SFLOAT_S8_UINT = 130,
	};

	enum class TextureUsageFlags : std::uint32_t
	{
		None = 0,
		TransferSrc = 1 << 0,
		TransferDst = 1 << 1,
		Sampled = 1 << 2,
		Storage = 1 << 3
	};

	inline TextureUsageFlags operator|(TextureUsageFlags a, TextureUsageFlags b)
	{
		return static_cast<TextureUsageFlags>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
	}

	enum class SamplerFilter : std::uint8_t
	{
		Nearest,
		Linear
	};

	enum class SamplerAddressMode : std::uint8_t
	{
		Repeat,
		ClampToEdge,
		ClampToBorder,
		MirrorRepeat
	};

	enum class SamplerMipmapMode : std::uint8_t
	{
		Nearest,
		Linear
	};

	enum class CompareOp : std::uint8_t
	{
		Never = 0,
		Less = 1,
		Equal = 2,
		LessOrEqual = 3,
		Greater = 4,
		NotEqual = 5,
		GreaterOrEqual = 6,
		Always = 7
	};

	enum class BorderColor : std::uint8_t
	{
		FloatTransparentBlack = 0,
		IntTransparentBlack = 1,
		FloatOpaqueBlack = 2,
		IntOpaqueBlack = 3,
		FloatOpaqueWhite = 4,
		IntOpaqueWhite = 5,
	};

	enum class BufferMemoryUsage : std::uint32_t
	{
		None = 0,
		GpuOnly,
		CpuOnly,
		CpuToGpu,
		GpuToCpu,
		CpuCopy,
		Auto
	};

	enum class BufferUsageFlags : std::uint32_t
	{
		None = 0,
		TransferSrc = 0x00000001,
		TransferDst = 0x00000002,
		Uniform = 0x00000010,
		Storage = 0x00000020,
		Index = 0x00000040,
		Vertex = 0x00000080,
	};

	enum class BufferAllocationFlags : std::uint32_t
	{
		None = 0,
		Mapped = 0x00000004,	// VMA_ALLOCATION_CREATE_MAPPED_BIT
		Dedicated = 0x00000001, // VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
	};

	inline BufferUsageFlags operator|(BufferUsageFlags a, BufferUsageFlags b)
	{
		return static_cast<BufferUsageFlags>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
	}

	inline BufferAllocationFlags operator|(BufferAllocationFlags a, BufferAllocationFlags b)
	{
		return static_cast<BufferAllocationFlags>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
	}

	inline bool operator&(BufferAllocationFlags a, BufferAllocationFlags b)
	{
		return (static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b)) != 0;
	}

}
