/*
 *	Better than Nothing
 *	Copyright (C) 2025 kakesinfo
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

#include "Module_Engine/ModelLoader.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <Module_Base/Logger.hpp>
#include <Module_Data/Graphics/ModelData.hpp>

namespace BetterThanNothing
{
	ModelData ModelLoader::Load(const std::string& path, bool preTransform)
	{
		ModelData model;

		tinyobj::attrib_t				 attrib;
		std::vector<tinyobj::shape_t>	 shapes;
		std::vector<tinyobj::material_t> materials;
		std::string						 warn;
		std::string						 err;

		std::filesystem::path parentFolder = std::filesystem::path(path).parent_path();

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), parentFolder.string().c_str()))
			throw std::runtime_error(err);

		if (!warn.empty())
			Logger::Log<WARNING>("TinyObjLoader warning: {}", warn);

		float minX = std::numeric_limits<float>::max();
		float minY = std::numeric_limits<float>::max();
		float minZ = std::numeric_limits<float>::max();

		float maxX = std::numeric_limits<float>::lowest();
		float maxY = std::numeric_limits<float>::lowest();
		float maxZ = std::numeric_limits<float>::lowest();

		for (const auto& shape : shapes)
		{
			MeshData														  mesh;
			std::unordered_map<VertexData, uint32_t, VertexHash, VertexEqual> uniqueVertices{};

			if (!shape.mesh.material_ids.empty())
			{
				mesh.materialId = shape.mesh.material_ids[0];

				if (mesh.materialId >= 0 && mesh.materialId < materials.size())
				{
					const auto& material = materials[mesh.materialId];

					if (!material.diffuse_texname.empty())
					{
						std::filesystem::path texPath = parentFolder / material.diffuse_texname;
						mesh.texturePath = texPath.string();
					}
				}
			}

			for (const auto& index : shape.mesh.indices)
			{
				VertexData vertex{};

				float rawX = attrib.vertices[3 * index.vertex_index + 0];
				float rawY = attrib.vertices[3 * index.vertex_index + 1];
				float rawZ = attrib.vertices[3 * index.vertex_index + 2];

				// Coordinate system transformation:
				// If preTransform is false, convert from Y-up (OBJ/Blender) to Z-up (engine) coordinate system.
				// This maps (X, Y, Z) -> (X, Z, -Y):
				//   - X remains the same.
				//   - Y becomes Z (up axis).
				//   - Z becomes -Y (to maintain right-handedness).
				if (preTransform)
				{
					vertex.position = {rawX, rawY, rawZ};
				}
				else
				{
					vertex.position = {rawX, rawZ, -rawY};
				}

				vertex.color = {1.0f, 1.0f, 1.0f};

				if (!attrib.texcoords.empty() && index.texcoord_index != -1)
				{
					vertex.uv = {attrib.texcoords[2 * index.texcoord_index + 0], 1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
				}
				else
				{
					vertex.uv = {0.0f, 0.0f};
				}

				// Load normals
				if (!attrib.normals.empty() && index.normal_index != -1)
				{
					auto xNormal = attrib.normals[3 * index.normal_index + 0];
					auto yNormal = attrib.normals[3 * index.normal_index + 1];
					auto zNormal = attrib.normals[3 * index.normal_index + 2];

					vertex.normal = preTransform ? glm::vec3{xNormal, yNormal, zNormal} : glm::vec3{xNormal, zNormal, -yNormal};
				}
				else
				{
					vertex.normal = {0.0f, 0.0f, 0.0f};
				}

				if (!uniqueVertices.contains(vertex))
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());

					if (vertex.position.x < minX)
						minX = vertex.position.x;
					if (vertex.position.y < minY)
						minY = vertex.position.y;
					if (vertex.position.z < minZ)
						minZ = vertex.position.z;

					if (vertex.position.x > maxX)
						maxX = vertex.position.x;
					if (vertex.position.y > maxY)
						maxY = vertex.position.y;
					if (vertex.position.z > maxZ)
						maxZ = vertex.position.z;

					mesh.vertices.push_back(vertex);
				}

				mesh.indices.push_back(uniqueVertices[vertex]);
			}
			model.meshes.push_back(mesh);
		}

		float centerX = (minX + maxX) / 2.0f;
		float centerY = (minY + maxY) / 2.0f;
		float centerZ = (minZ + maxZ) / 2.0f;

		for (auto& mesh : model.meshes)
		{
			for (auto& vert : mesh.vertices)
			{
				vert.position.x -= centerX;
				vert.position.y -= centerY;
				vert.position.z -= centerZ;
			}
		}

		return model;
	}
}
