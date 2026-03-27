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

namespace BetterThanNothing
{
	ResourceImporter::ResourceImporter()
	{
		m_textureImporter = std::make_unique<TextureImporter>();
		m_objImporter = std::make_unique<ObjImporter>();
		m_gltfImporter = std::make_unique<GltfImporter>();
	}

	ResourceImporter::~ResourceImporter() {}

	ImportedTexture ResourceImporter::ImportTexture(const std::string& input) const
	{
		return m_textureImporter->Import(input);
	}

	ImportedModel ResourceImporter::ImportModel(const std::string& path, bool preTransform) const
	{
		if (path.find(".obj") != std::string::npos)
			return m_objImporter->Import({path, preTransform});

		if (path.find(".gltf") != std::string::npos)
			return m_gltfImporter->Import({path, preTransform});

		throw std::runtime_error("Unsupported file type");
	}

	ImportedShader ResourceImporter::ImportShader(const std::string& path) const
	{
		return m_shaderImporter->Import(path);
	}
}
