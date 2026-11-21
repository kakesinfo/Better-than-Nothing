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

#include "Common/EShaderType.hpp"

#include <Data/Graphics/VertexData.hpp>
#include <Math/Matrix4.hpp>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace BetterThanNothing
{
	using ImportedId = uint32_t;

	enum class EImportedAlphaMode : uint8_t
	{
		NONE = 0,
		Opaque,
		Mask,
		Blend
	};

	struct ImportedShader
	{
		std::string			  path;
		EShaderType			  type = EShaderType::Vertex;
		std::vector<uint32_t> spirv;
	};

	struct ImportedTexture
	{
		std::string				   name;
		std::optional<std::string> path;
		std::vector<uint8_t>	   data;
		uint32_t				   width = 0;
		uint32_t				   height = 0;
		uint32_t				   channels = 0;
		uint32_t				   size = 0;
	};

	struct ImportedMaterial
	{
		uint32_t				   id = 0;
		EImportedAlphaMode		   alphaMode = EImportedAlphaMode::Opaque;
		float					   alphaCutoff = 0.0f;
		Vector4					   baseColorFactor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		float					   metallicFactor = 0.0f;
		float					   roughnessFactor = 0.65f;
		std::optional<std::string> diffuseTexturePath;
		std::optional<std::string> normalTexturePath;
		std::optional<std::string> metallicRoughnessTexturePath;
	};

	struct ImportedMesh
	{
		uint32_t id = 0;

		std::vector<VertexData> vertices;
		std::vector<uint32_t>	indices;

		std::optional<ImportedId> materialId;

		Vector3 min = Vector3::Zero;
		Vector3 max = Vector3::Zero;
	};

	struct ImportedNode
	{
		std::string				name;
		Matrix4					transform = Matrix4::Identity;
		std::vector<ImportedId> meshes;
		std::vector<ImportedId> children;
	};

	struct ImportedModel
	{
		std::string path;

		std::vector<ImportedMesh>	  meshes;
		std::vector<ImportedMaterial> materials;
		std::vector<ImportedNode>	  nodes;
		std::vector<ImportedId>		  rootNodes;
	};
}
