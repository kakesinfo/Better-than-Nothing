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

#include "ResourceImporter_Types.hpp"

#include <memory>

namespace BetterThanNothing
{
	class TextureImporter;
	class ObjImporter;
	class GltfImporter;

	class ResourceImporter
	{
	public:
		ResourceImporter();
		~ResourceImporter();

		[[nodiscard]] ImportedTexture ImportTexture(const std::string& input) const;
		[[nodiscard]] ImportedModel ImportModel(const std::string& path, bool preTransform = true) const;

	private:
		std::unique_ptr<TextureImporter> m_textureImporter;
		std::unique_ptr<ObjImporter> m_objImporter;
		std::unique_ptr<GltfImporter> m_gltfImporter;
	};
}
