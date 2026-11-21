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

#include <ResourceImporter/Importer.hpp>
#include <ResourceImporter/ResourceImporter.hpp>
#include <ResourceImporter/ResourceImporter_Types.hpp>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <glslang/Include/glslang_c_interface.h>

namespace BetterThanNothing
{
	ResourceImporter::ResourceImporter()
	{
		glslang_initialize_process();
		m_textureImporter = std::make_unique<TextureImporter>();
		m_objImporter = std::make_unique<ObjImporter>();
		m_gltfImporter = std::make_unique<GltfImporter>();
		m_shaderImporter = std::make_unique<ShaderImporter>();
	}

	ResourceImporter::~ResourceImporter()
	{
		glslang_finalize_process();
	}

	ImportedTexture ResourceImporter::ImportTexture(const std::string& input, const uint32_t maxDimension) const
	{
		return m_textureImporter->Import({input, maxDimension});
	}

	ImportedModel ResourceImporter::ImportModel(const std::string& path, bool preTransform) const
	{
		std::string ext = std::filesystem::path(path).extension().string();
		std::ranges::transform(ext, ext.begin(), [](const unsigned char c) { return static_cast<char>(std::tolower(c)); });

		if (ext == ".obj")
			return m_objImporter->Import({path, preTransform});

		if (ext == ".gltf" || ext == ".glb")
			return m_gltfImporter->Import({path, preTransform});

		throw std::runtime_error("Unsupported model file type: " + path);
	}

	ImportedShader ResourceImporter::ImportShader(const std::string& path) const
	{
		return m_shaderImporter->Import(path);
	}
}
