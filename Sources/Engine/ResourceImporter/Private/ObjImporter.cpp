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

#include <Common/AssetPath.hpp>
#include <Common/Logger.hpp>
#include <ResourceImporter/ResourceImporter_Types.hpp>
#include <ResourceImporter/Importer.hpp>
#include <algorithm>
#include <cmath>
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

		const int materialId = shape.mesh.material_ids.at(0);

		if (materialId < 0 || static_cast<size_t>(materialId) >= materials.size())
			return std::nullopt;

		const auto& material = materials[materialId];

		if (!material.diffuse_texname.empty())
		{
			const std::filesystem::path texPath = parentFolder / material.diffuse_texname;
			importedMaterial.diffuseTexturePath = texPath.string();
		}

		std::string normalTexName = material.normal_texname;
		if (normalTexName.empty())
			normalTexName = material.bump_texname;
		if (normalTexName.empty() && !material.displacement_texname.empty())
			normalTexName = material.displacement_texname;

		if (!normalTexName.empty())
		{
			const std::filesystem::path texPath = parentFolder / normalTexName;
			importedMaterial.normalTexturePath = texPath.string();
		}

		std::string roughnessTexName = material.roughness_texname;
		if (roughnessTexName.empty())
			roughnessTexName = material.specular_highlight_texname;
		if (!roughnessTexName.empty())
		{
			const std::filesystem::path texPath = parentFolder / roughnessTexName;
			importedMaterial.metallicRoughnessTexturePath = texPath.string();
		}

		// Legacy OBJ exporters (including Crytek Sponza) write `d 0.000000` for opaque materials
		const float alpha = (material.dissolve <= 0.0f) ? 1.0f : material.dissolve;

		const float diffuseSum = material.diffuse[0] + material.diffuse[1] + material.diffuse[2];
		if (!material.diffuse_texname.empty() || diffuseSum <= 0.0f)
			importedMaterial.baseColorFactor = Vector4(1.0f, 1.0f, 1.0f, alpha);
		else
			importedMaterial.baseColorFactor = Vector4(material.diffuse[0], material.diffuse[1], material.diffuse[2], alpha);

		importedMaterial.metallicFactor = material.metallic;
		if (material.roughness > 0.0f)
			importedMaterial.roughnessFactor = material.roughness;
		else if (material.shininess > 0.0f)
			importedMaterial.roughnessFactor = std::clamp(std::sqrt(2.0f / (material.shininess + 2.0f)), 0.15f, 0.95f);
		else
			importedMaterial.roughnessFactor = 0.85f;

		if (!material.alpha_texname.empty())
		{
			importedMaterial.alphaMode = EImportedAlphaMode::Mask;
			importedMaterial.alphaCutoff = 0.5f;
		}

		importedMaterial.id = static_cast<uint32_t>(materialId);

		return importedMaterial;
	}

	ImportedModel ObjImporter::Import(const ModelImportInfo& input)
	{
		ImportedModel model;

		const std::string resolvedPath = AssetPath::Resolve(input.path);

		std::unordered_map<uint32_t, uint32_t> materialCache;

		tinyobj::attrib_t				 attrib;
		std::vector<tinyobj::shape_t>	 shapes;
		std::vector<tinyobj::material_t> materials;
		std::string						 warn;
		std::string						 err;

		std::filesystem::path parentFolder = std::filesystem::path(resolvedPath).parent_path();

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, resolvedPath.c_str(), parentFolder.string().c_str()))
			throw std::runtime_error(err);

		if (!warn.empty())
			Logger::Log<WARNING>("TinyObjLoader warning: {}", warn);

		float minX = std::numeric_limits<float>::max();
		float minY = std::numeric_limits<float>::max();
		float minZ = std::numeric_limits<float>::max();

		float maxX = std::numeric_limits<float>::lowest();
		float maxY = std::numeric_limits<float>::lowest();
		float maxZ = std::numeric_limits<float>::lowest();

		model.path = input.path;

		for (const auto& shape : shapes)
		{
			if (shape.mesh.indices.empty())
				continue;

			const auto	 currentMeshIndex = static_cast<uint32_t>(model.meshes.size());
			ImportedMesh currentMesh{};
			currentMesh.id = currentMeshIndex;
			currentMesh.vertices.reserve(shape.mesh.indices.size());

			ImportedNode currentNode{};
			currentNode.name = shape.name;

			const std::optional<ImportedMaterial> currentMaterial = ImportMaterial(parentFolder, shape, materials);
			if (currentMaterial.has_value())
			{
				uint32_t rawId = currentMaterial.value().id;
				if (!materialCache.contains(rawId))
				{
					materialCache[rawId] = static_cast<uint32_t>(model.materials.size());
					model.materials.push_back(currentMaterial.value());
				}
				currentMesh.materialId = materialCache[rawId];
			}

			for (const auto& index : shape.mesh.indices)
			{
				VertexData vertex{};

				const float rawX = attrib.vertices[3 * index.vertex_index + 0];
				const float rawY = attrib.vertices[3 * index.vertex_index + 1];
				const float rawZ = attrib.vertices[3 * index.vertex_index + 2];

				// Coordinate system transformation:
				// If preTransform is false, convert from Y-up (OBJ/Blender) to Z-up (engine) coordinate system.
				if (input.preTransform)
					vertex.position = {rawX, rawY, rawZ};
				else
					vertex.position = {rawX, rawZ, -rawY};

				vertex.color = {1.0f, 1.0f, 1.0f};

				if (!attrib.texcoords.empty() && index.texcoord_index >= 0)
					vertex.uv = {attrib.texcoords[2 * index.texcoord_index + 0], 1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
				else
					vertex.uv = {0.0f, 0.0f};

				if (!attrib.normals.empty() && index.normal_index >= 0)
				{
					const auto xNormal = attrib.normals[3 * index.normal_index + 0];
					const auto yNormal = attrib.normals[3 * index.normal_index + 1];
					const auto zNormal = attrib.normals[3 * index.normal_index + 2];

					vertex.normal = input.preTransform ? Vector3{xNormal, yNormal, zNormal} : Vector3{xNormal, zNormal, -yNormal};
				}
				else
				{
					vertex.normal = {0.0f, 1.0f, 0.0f};
				}

				minX = std::min(minX, vertex.position.GetX());
				minY = std::min(minY, vertex.position.GetY());
				minZ = std::min(minZ, vertex.position.GetZ());
				maxX = std::max(maxX, vertex.position.GetX());
				maxY = std::max(maxY, vertex.position.GetY());
				maxZ = std::max(maxZ, vertex.position.GetZ());

				currentMesh.vertices.push_back(vertex);
			}

			const auto nodeIdx = static_cast<ImportedId>(model.nodes.size());
			currentNode.meshes.push_back(currentMeshIndex);
			currentNode.transform = Matrix4::Identity;

			model.meshes.push_back(std::move(currentMesh));
			model.nodes.push_back(std::move(currentNode));
			model.rootNodes.push_back(nodeIdx);
		}

		if (model.meshes.empty())
			return model;

		const Vector3 center = Vector3(minX + maxX, minY + maxY, minZ + maxZ) / 2.0f;
		for (auto& mesh : model.meshes)
		{
			const size_t vertexCountIn = mesh.vertices.size();
			if (vertexCountIn == 0)
				continue;

			std::vector<unsigned int> remap(vertexCountIn);
			const size_t uniqueVertexCount = meshopt_generateVertexRemap(remap.data(), nullptr, vertexCountIn, mesh.vertices.data(), vertexCountIn, sizeof(VertexData));

			std::vector<VertexData> newVertices(uniqueVertexCount);
			std::vector<uint32_t>	newIndices(vertexCountIn);

			meshopt_remapVertexBuffer(newVertices.data(), mesh.vertices.data(), vertexCountIn, sizeof(VertexData), remap.data());
			meshopt_remapIndexBuffer(newIndices.data(), nullptr, vertexCountIn, remap.data());

			meshopt_optimizeVertexCache(newIndices.data(), newIndices.data(), vertexCountIn, uniqueVertexCount);
			meshopt_optimizeOverdraw(newIndices.data(), newIndices.data(), vertexCountIn, &newVertices[0].position.x(), uniqueVertexCount, sizeof(VertexData), 1.05f);
			meshopt_optimizeVertexFetch(newVertices.data(), newIndices.data(), vertexCountIn, newVertices.data(), uniqueVertexCount, sizeof(VertexData));

			Vector3 meshMin(std::numeric_limits<float>::max());
			Vector3 meshMax(std::numeric_limits<float>::lowest());
			for (auto& vert : newVertices)
			{
				vert.position -= center;
				meshMin = Vector3::Min(meshMin, vert.position);
				meshMax = Vector3::Max(meshMax, vert.position);
			}

			mesh.min = meshMin;
			mesh.max = meshMax;
			mesh.vertices = std::move(newVertices);
			mesh.indices = std::move(newIndices);
		}

		return model;
	};
}
