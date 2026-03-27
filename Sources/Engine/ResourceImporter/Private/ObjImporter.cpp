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

#define TINYOBJLOADER_IMPLEMENTATION

#include <Common/Logger.hpp>
#include <ResourceImporter/ResourceImporter_Types.hpp>
#include <ResourceImporter/Importer.hpp>
#include <cstring>
#include <filesystem>
#include <meshoptimizer.h>
#include <tiny_obj_loader.h>

namespace BetterThanNothing
{
	static std::optional<ImportedMaterial> ImportMaterial(const std::filesystem::path& parentFolder, const tinyobj::shape_t& shape, const std::vector<tinyobj::material_t>& materials)
	{
		ImportedMaterial importedMaterial;
		importedMaterial.alphaMode = EImportedAlphaMode::Opaque;
		importedMaterial.alphaCutoff = 0.0f;

		if (shape.mesh.material_ids.empty())
			return std::nullopt;

		const int materialId = shape.mesh.material_ids.at(0); // We assume that there is only one material

		if (materialId < 0 || materialId >= materials.size())
			return std::nullopt;

		const auto& material = materials[materialId];

		if (!material.diffuse_texname.empty())
		{
			const std::filesystem::path texPath = parentFolder / material.diffuse_texname;
			importedMaterial.diffuseTexturePath = texPath.string();
		}

		if (!material.normal_texname.empty())
		{

			const std::filesystem::path texPath = parentFolder / material.normal_texname;
			importedMaterial.normalTexturePath = texPath.string();
		}

		importedMaterial.id = materialId;

		return importedMaterial;
	}

	ImportedModel ObjImporter::Import(const ModelImportInfo& input)
	{
		ImportedModel model;

		std::unordered_map<uint32_t, uint32_t> materialCache;

		tinyobj::attrib_t				 attrib;
		std::vector<tinyobj::shape_t>	 shapes;
		std::vector<tinyobj::material_t> materials;
		std::string						 warn;
		std::string						 err;

		std::filesystem::path parentFolder = std::filesystem::path(input.path).parent_path();

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, input.path.c_str(), parentFolder.string().c_str()))
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
			uint32_t	 currentMeshIndex = model.meshes.size();
			ImportedMesh currentMesh;
			ImportedNode currentNode;

			const std::optional<ImportedMaterial> currentMaterial = ImportMaterial(parentFolder, shape, materials);
			if (currentMaterial.has_value())
			{
				uint32_t rawId = currentMaterial.value().id;
				if (!materialCache.contains(rawId))
				{
					materialCache[rawId] = model.materials.size();
					model.materials.push_back(currentMaterial.value());
				}
				currentMesh.materialId = materialCache[rawId];
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
				if (input.preTransform)
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

				if (!attrib.normals.empty() && index.normal_index != -1)
				{
					auto xNormal = attrib.normals[3 * index.normal_index + 0];
					auto yNormal = attrib.normals[3 * index.normal_index + 1];
					auto zNormal = attrib.normals[3 * index.normal_index + 2];

					vertex.normal = input.preTransform ? Vector3{xNormal, yNormal, zNormal} : Vector3{xNormal, zNormal, -yNormal};
				}
				else
				{
					vertex.normal = {0.0f, 0.0f, 0.0f};
				}

				if (vertex.position.GetX() < minX)
					minX = vertex.position.GetX();
				if (vertex.position.GetY() < minY)
					minY = vertex.position.GetY();
				if (vertex.position.GetZ() < minZ)
					minZ = vertex.position.GetZ();

				if (vertex.position.GetX() > maxX)
					maxX = vertex.position.GetX();
				if (vertex.position.GetY() > maxY)
					maxY = vertex.position.GetY();
				if (vertex.position.GetZ() > maxZ)
					maxZ = vertex.position.GetZ();

				currentMesh.vertices.push_back(vertex);
			}

			currentNode.mesh = currentMeshIndex;
			currentNode.transform = Matrix4::Identity;

			model.meshes.push_back(currentMesh);
			model.children.push_back(currentNode);
		}

		const Vector3 center = Vector3(minX + maxX, minY + maxY, minZ + maxZ) / 2.0f;
		for (auto& mesh : model.meshes)
		{
			const size_t vertex_count_in = mesh.vertices.size();

			std::vector<unsigned int> remap(vertex_count_in);
			const size_t unique_vertex_count = meshopt_generateVertexRemap(remap.data(), nullptr, vertex_count_in, mesh.vertices.data(), vertex_count_in, sizeof(VertexData));

			std::vector<VertexData> new_vertices(unique_vertex_count);
			std::vector<uint32_t>	new_indices(vertex_count_in);

			meshopt_remapVertexBuffer(new_vertices.data(), mesh.vertices.data(), vertex_count_in, sizeof(VertexData), remap.data());
			meshopt_remapIndexBuffer(new_indices.data(), nullptr, vertex_count_in, remap.data());

			meshopt_optimizeVertexCache(new_indices.data(), new_indices.data(), vertex_count_in, unique_vertex_count);
			meshopt_optimizeOverdraw(new_indices.data(), new_indices.data(), vertex_count_in, &new_vertices[0].position.x(), unique_vertex_count, sizeof(VertexData), 1.05f);
			meshopt_optimizeVertexFetch(new_vertices.data(), new_indices.data(), vertex_count_in, new_vertices.data(), unique_vertex_count, sizeof(VertexData));

			mesh.vertices = new_vertices;
			mesh.indices = new_indices;

			for (auto& vert : mesh.vertices)
				vert.position -= center;
		}

		for (auto& node : model.children)
			node.transform = node.transform.Translate(center);

		return model;
	};
}
