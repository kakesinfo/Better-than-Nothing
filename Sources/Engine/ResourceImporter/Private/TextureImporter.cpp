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
#include <vector>

namespace BetterThanNothing
{
	static void DownsampleHalfRGBA(std::vector<uint8_t>& data, uint32_t& width, uint32_t& height)
	{
		const uint32_t newWidth = std::max(1u, width / 2u);
		const uint32_t newHeight = std::max(1u, height / 2u);

		std::vector<uint8_t> dst(static_cast<size_t>(newWidth) * newHeight * 4u);
		for (uint32_t y = 0; y < newHeight; ++y)
		{
			const uint32_t sy0 = std::min(y * 2u, height - 1u);
			const uint32_t sy1 = std::min(sy0 + 1u, height - 1u);
			const uint8_t* row0 = data.data() + static_cast<size_t>(sy0) * width * 4u;
			const uint8_t* row1 = data.data() + static_cast<size_t>(sy1) * width * 4u;
			uint8_t*	   outRow = dst.data() + static_cast<size_t>(y) * newWidth * 4u;

			for (uint32_t x = 0; x < newWidth; ++x)
			{
				const uint32_t sx0 = std::min(x * 2u, width - 1u) * 4u;
				const uint32_t sx1 = std::min(x * 2u + 1u, width - 1u) * 4u;
				const uint32_t dx = x * 4u;

				for (uint32_t c = 0; c < 4u; ++c)
				{
					const uint32_t sum =
						static_cast<uint32_t>(row0[sx0 + c]) +
						static_cast<uint32_t>(row0[sx1 + c]) +
						static_cast<uint32_t>(row1[sx0 + c]) +
						static_cast<uint32_t>(row1[sx1 + c]) + 2u;
					outRow[dx + c] = static_cast<uint8_t>(sum >> 2u);
				}
			}
		}

		data = std::move(dst);
		width = newWidth;
		height = newHeight;
	}

	ImportedTexture TextureImporter::Import(const TextureImportInfo& input)
	{
		const std::string resolvedPath = AssetPath::Resolve(input.path);

		ImportedTexture texture;
		int				width = 0, height = 0;

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

		if (input.maxDimension > 0u)
		{
			while (texture.width > input.maxDimension || texture.height > input.maxDimension)
				DownsampleHalfRGBA(texture.data, texture.width, texture.height);
			texture.size = texture.width * texture.height * texture.channels;
		}

		return texture;
	}
}
