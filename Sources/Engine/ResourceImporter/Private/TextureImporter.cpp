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

#define STB_IMAGE_IMPLEMENTATION

#include <Common/AssetPath.hpp>
#include <ResourceImporter/Importer.hpp>
#include <ResourceImporter/ResourceImporter_Types.hpp>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <stb_image.h>

namespace BetterThanNothing
{
	void DownsampleHalfRGBA(const uint8_t* src, const uint32_t srcW, const uint32_t srcH, std::vector<uint8_t>& dst, uint32_t& dstW, uint32_t& dstH)
	{
		dstW = std::max(1u, srcW / 2);
		dstH = std::max(1u, srcH / 2);
		dst.resize(static_cast<size_t>(dstW) * dstH * 4);

		for (uint32_t y = 0; y < dstH; ++y)
		{
			const uint32_t y0 = y * 2;
			const uint32_t y1 = std::min(y0 + 1, srcH - 1);
			for (uint32_t x = 0; x < dstW; ++x)
			{
				const uint32_t x0 = x * 2;
				const uint32_t x1 = std::min(x0 + 1, srcW - 1);

				const uint8_t* p00 = src + (static_cast<size_t>(y0) * srcW + x0) * 4;
				const uint8_t* p10 = src + (static_cast<size_t>(y0) * srcW + x1) * 4;
				const uint8_t* p01 = src + (static_cast<size_t>(y1) * srcW + x0) * 4;
				const uint8_t* p11 = src + (static_cast<size_t>(y1) * srcW + x1) * 4;

				uint8_t* out = dst.data() + (static_cast<size_t>(y) * dstW + x) * 4;
				for (int c = 0; c < 4; ++c)
				{
					const uint32_t sum = static_cast<uint32_t>(p00[c]) + p10[c] + p01[c] + p11[c];
					out[c] = static_cast<uint8_t>((sum + 2) >> 2);
				}
			}
		}
	}

	ImportedTexture TextureImporter::Import(const TextureImportInfo& input)
	{
		const std::string resolvedPath = AssetPath::Resolve(input.path);

		ImportedTexture texture;
		int				width, height;

		uint8_t* pixels = stbi_load(resolvedPath.c_str(), &width, &height, nullptr, STBI_rgb_alpha);

		if (!pixels)
		{
			std::stringstream ss;
			ss << "Failed to load texture image from path: " << resolvedPath << " - " << stbi_failure_reason() << std::endl;
			throw std::runtime_error(ss.str());
		}

		texture.width = static_cast<uint32_t>(width);
		texture.height = static_cast<uint32_t>(height);
		texture.channels = STBI_rgb_alpha;
		texture.size = texture.width * texture.height * texture.channels;

		texture.data.resize(texture.size);
		std::memcpy(texture.data.data(), pixels, texture.size);

		stbi_image_free(pixels);

		while (input.maxDimension > 0 && (texture.width > input.maxDimension || texture.height > input.maxDimension))
		{
			std::vector<uint8_t> downsampled;
			uint32_t			 nextW = 0;
			uint32_t			 nextH = 0;
			DownsampleHalfRGBA(texture.data.data(), texture.width, texture.height, downsampled, nextW, nextH);
			texture.data = std::move(downsampled);
			texture.width = nextW;
			texture.height = nextH;
			texture.size = static_cast<uint32_t>(texture.data.size());
		}

		return texture;
	};
}
