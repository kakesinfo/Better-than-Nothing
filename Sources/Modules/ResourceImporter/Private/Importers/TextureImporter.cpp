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

#include <ResourceImporter/ResourceImporter_Types.hpp>
#include <ResourceImporter/TextureImporter.hpp>
#include <cstring>
#include <sstream>
#include <stb_image.h>

namespace BetterThanNothing
{
	ImportedTexture TextureImporter::Import(const std::string& input)
	{
		ImportedTexture texture;
		int				width, height;

		uint8_t* pixels = stbi_load(input.c_str(), &width, &height, nullptr, STBI_rgb_alpha);

		if (!pixels)
		{
			std::stringstream ss;
			ss << "Failed to load texture image from path: " << input << " - " << stbi_failure_reason() << std::endl;
			throw std::runtime_error(ss.str());
		}

		texture.width = static_cast<uint32_t>(width);
		texture.height = static_cast<uint32_t>(height);
		texture.channels = STBI_rgb_alpha;
		texture.size = texture.width * texture.height * texture.channels;

		texture.data.resize(texture.size);
		std::memcpy(texture.data.data(), pixels, texture.size);

		stbi_image_free(pixels);

		return texture;
	};
}
