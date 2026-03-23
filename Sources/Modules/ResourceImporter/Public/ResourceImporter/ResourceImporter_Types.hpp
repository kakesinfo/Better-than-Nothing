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

#include <Module_Data/Graphics/VertexData.hpp>
#include <Module_Math/Matrix4.hpp>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace BetterThanNothing
{
	using ImportedId = uint32_t;

	struct ImportedTexture
	{
		std::string				   name;
		std::optional<std::string> path;
		std::vector<uint8_t>	   data;

		uint32_t width;
		uint32_t height;
		uint32_t channels;
		uint32_t size;
	};

	struct ImportedMaterial
	{
		uint32_t				   id;
		std::optional<std::string> diffuseTexturePath;
		std::optional<std::string> normalTexturePath;
	};

	struct ImportedMesh
	{
		uint32_t id;

		std::vector<VertexData> vertices;
		std::vector<uint32_t>	indices;

		std::optional<ImportedId> materialId;

		Vector3 min;
		Vector3 max;
	};

	struct ImportedNode
	{
		Matrix4					  transform;
		std::optional<ImportedId> mesh;
	};

	struct ImportedModel
	{
		std::string path;

		std::vector<ImportedMesh>	  meshes;
		std::vector<ImportedMaterial> materials;

		std::vector<ImportedNode> children;
	};
}
