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

#include <ResourceImporter/ResourceImporter_Types.hpp>

#define DECLARE_IMPORTER(name, inputType, outputType) \
	class name final : public Importer<inputType, outputType> \
	{ \
		public: \
		name() = default; \
		~name() override = default; \
		outputType Import(const inputType& input) override; \
	}

namespace BetterThanNothing
{
	template <typename InputType, typename OutputType>
	class Importer
	{
	public:
		virtual ~Importer() = default;
		virtual OutputType Import(const InputType& input) = 0;
	};

	struct ModelImportInfo
	{
		std::string path;
		bool		preTransform = true;
	};

	DECLARE_IMPORTER(ObjImporter, ModelImportInfo, ImportedModel);
	DECLARE_IMPORTER(GltfImporter, ModelImportInfo, ImportedModel);
	DECLARE_IMPORTER(TextureImporter, std::string, ImportedTexture);
	DECLARE_IMPORTER(ShaderImporter, std::string, ImportedShader);
}
