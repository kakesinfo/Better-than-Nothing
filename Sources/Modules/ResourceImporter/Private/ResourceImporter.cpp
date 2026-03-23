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

#include <ResourceImporter/ResourceImporter.hpp>

#include "ResourceImporter/ObjModelImporter.hpp"

#include <ResourceImporter/ResourceImporter_Types.hpp>
#include <ResourceImporter/TextureImporter.hpp>

namespace BetterThanNothing
{
	ResourceImporter::ResourceImporter()
	{
		m_textureImporter = std::make_unique<TextureImporter>();
		m_modelImporter = std::make_unique<ObjModelImporter>();
	}

	ResourceImporter::~ResourceImporter()
	{

	}

	ImportedTexture ResourceImporter::ImportTexture(const std::string& input) const
	{
		return m_textureImporter->Import(input);
	}

	ImportedModel ResourceImporter::ImportModel(const std::string& path, bool preTransform) const
	{
		return m_modelImporter->Import({path, preTransform});
	}
}
