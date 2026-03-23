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
#include <unordered_map>

namespace BetterThanNothing
{
	class TextureImporter;

	class ResourceImporter
	{
	public:
		ResourceImporter();
		~ResourceImporter();

		ImportedTexture ImportTexture(const std::string& input) const;

	private:
		std::unique_ptr<TextureImporter> m_textureImporter;
	};
}
