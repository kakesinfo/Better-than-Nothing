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
#include <Math/Vector4.hpp>
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
		std::string		  path;
		EShaderType		  type;
		std::vector<char> data;
	};

	struct ImportedTexture
	{
		std::string				   name;
		std::optional<std::string> path;
		std::vector<uint8_t>	   data;
		uint32_t				   width;
		uint32_t				   height;
		uint32_t				   channels;
		uint32_t				   size;
	};

	struct ImportedMaterial
	{
		uint32_t				   id{0};
		std::string				   name;
		EImportedAlphaMode		   alphaMode{EImportedAlphaMode::Opaque};
		float					   alphaCutoff{0.5f};

		Vector4					   baseColorFactor{1.0f, 1.0f, 1.0f, 1.0f};
		float					   metallicFactor{1.0f};
		float					   roughnessFactor{1.0f};
		Vector3					   emissiveFactor{0.0f, 0.0f, 0.0f};

		std::optional<std::string> diffuseTexturePath;
		std::optional<std::string> normalTexturePath;
		std::optional<std::string> metallicRoughnessTexturePath;
		std::optional<std::string> occlusionTexturePath;
		std::optional<std::string> emissiveTexturePath;
	};

	struct ImportedMesh
	{
		uint32_t id{0};
		std::string name;

		std::vector<VertexData> vertices;
		std::vector<uint32_t>	indices;

		std::optional<ImportedId> materialId;

		Vector3 min;
		Vector3 max;
	};

	struct ImportedNode
	{
		std::string				  name;
		Matrix4					  transform{Matrix4::Identity};
		std::vector<ImportedId>   meshes;
		std::vector<uint32_t>	  children;
	};

	struct ImportedModel
	{
		std::string path;

		std::vector<ImportedMesh>	  meshes;
		std::vector<ImportedMaterial> materials;

		std::vector<ImportedNode> children;
		std::vector<uint32_t>	  rootNodes;
	};
}
