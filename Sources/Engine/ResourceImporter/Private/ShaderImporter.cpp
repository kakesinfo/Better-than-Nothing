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
#include <ResourceImporter/ResourceImporter_Types.hpp>
#include <fstream>
#include <sstream>

namespace BetterThanNothing
{
	ImportedShader ShaderImporter::Import(const std::string& input)
	{

		std::ifstream file(input, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			throw std::runtime_error("failed to open file!");

		const std::streampos endPos = file.tellg();
		if (endPos < 0)
			throw std::runtime_error("failed to determine file size!");

		const std::streamsize fileSize = endPos;

		ImportedShader importedShader;
		importedShader.path = input;
		importedShader.data.resize(fileSize + 1);
		if (fileSize > 0)
		{
			file.seekg(0, std::ios::beg);
			file.read(importedShader.data.data(), fileSize);
			if (!file)
				throw std::runtime_error("failed to read file!");
		}

		importedShader.data[fileSize] = '\0';
		file.close();

		return importedShader;
	};
}
