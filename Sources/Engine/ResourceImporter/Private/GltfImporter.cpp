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

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE

#include <Common/AssetPath.hpp>
#include <Common/Logger.hpp>
#include <ResourceImporter/Importer.hpp>
#include <ResourceImporter/ResourceImporter_Types.hpp>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <functional>
#include <limits>
#include <map>
#include <meshoptimizer.h>
#include <tiny_gltf.h>

namespace BetterThanNothing
{
	static EImportedAlphaMode Internal_RetrieveAlphaMode(const std::string& value)
	{
		if (value == "OPAQUE" || value.empty())
			return EImportedAlphaMode::Opaque;
		if (value == "MASK")
			return EImportedAlphaMode::Mask;
		if (value == "BLEND")
			return EImportedAlphaMode::Blend;
		throw std::runtime_error("Unknown Alpha Mode value: " + value);
	}

	static bool IsLikelyFoliageName(std::string s)
	{
		std::ranges::transform(s, s.begin(), [](const unsigned char c) { return static_cast<char>(std::tolower(c)); });
		return s.find("leaf") != std::string::npos || s.find("leaves") != std::string::npos || s.find("foliage") != std::string::npos;
	}

	static void OptimizeAndBoundMesh(ImportedMesh& mesh)
	{
		const bool	 isIndexed = !mesh.indices.empty();
		const size_t vertexCountIn = mesh.vertices.size();
		const size_t indexCountIn = isIndexed ? mesh.indices.size() : vertexCountIn;
		if (vertexCountIn == 0 || indexCountIn == 0)
			return;

		std::vector<unsigned int> remap(vertexCountIn);
		const size_t			  uniqueVertexCount = meshopt_generateVertexRemap(
			remap.data(),
			isIndexed ? mesh.indices.data() : nullptr,
			indexCountIn,
			mesh.vertices.data(),
			vertexCountIn,
			sizeof(VertexData)
		);

		std::vector<VertexData> newVertices(uniqueVertexCount);
		std::vector<uint32_t>	newIndices(indexCountIn);

		meshopt_remapVertexBuffer(newVertices.data(), mesh.vertices.data(), vertexCountIn, sizeof(VertexData), remap.data());
		meshopt_remapIndexBuffer(newIndices.data(), isIndexed ? mesh.indices.data() : nullptr, indexCountIn, remap.data());

		meshopt_optimizeVertexCache(newIndices.data(), newIndices.data(), indexCountIn, uniqueVertexCount);
		meshopt_optimizeOverdraw(newIndices.data(), newIndices.data(), indexCountIn, &newVertices[0].position.x(), uniqueVertexCount, sizeof(VertexData), 1.05f);
		meshopt_optimizeVertexFetch(newVertices.data(), newIndices.data(), indexCountIn, newVertices.data(), uniqueVertexCount, sizeof(VertexData));

		Vector3 meshMin(std::numeric_limits<float>::max());
		Vector3 meshMax(std::numeric_limits<float>::lowest());
		for (const auto& vert : newVertices)
		{
			meshMin = Vector3::Min(meshMin, vert.position);
			meshMax = Vector3::Max(meshMax, vert.position);
		}

		mesh.min = meshMin;
		mesh.max = meshMax;
		mesh.vertices = std::move(newVertices);
		mesh.indices = std::move(newIndices);
	}

	ImportedModel GltfImporter::Import(const ModelImportInfo& input)
	{
		ImportedModel importedModel;
		importedModel.path = input.path;

		tinygltf::Model	   gltfModel;
		tinygltf::TinyGLTF loader;
		std::string		   err;
		std::string		   warn;

		const std::string	  resolvedPath = AssetPath::Resolve(input.path);
		std::filesystem::path filePath(resolvedPath);
		std::string			  ext = filePath.extension().string();
		std::ranges::transform(ext, ext.begin(), [](const unsigned char c) { return static_cast<char>(std::tolower(c)); });

		const bool ret = (ext == ".glb") ? loader.LoadBinaryFromFile(&gltfModel, &err, &warn, resolvedPath)
										 : loader.LoadASCIIFromFile(&gltfModel, &err, &warn, resolvedPath);

		if (!warn.empty())
			Logger::Log<WARNING>("TinyGLTF warning: {}", warn);

		if (!err.empty())
			throw std::runtime_error("TinyGLTF error: " + err);

		if (!ret)
			throw std::runtime_error("Failed to parse glTF file: " + input.path);

		importedModel.materials.reserve(gltfModel.materials.size());
		for (size_t matIdx = 0; matIdx < gltfModel.materials.size(); ++matIdx)
		{
			const auto&		 gltfMat = gltfModel.materials[matIdx];
			ImportedMaterial mat{};
			mat.id = static_cast<uint32_t>(matIdx);
			mat.alphaMode = Internal_RetrieveAlphaMode(gltfMat.alphaMode);
			mat.alphaCutoff = static_cast<float>(gltfMat.alphaCutoff);
			mat.metallicFactor = static_cast<float>(gltfMat.pbrMetallicRoughness.metallicFactor);
			mat.roughnessFactor = static_cast<float>(gltfMat.pbrMetallicRoughness.roughnessFactor);

			if (gltfMat.pbrMetallicRoughness.baseColorFactor.size() == 4)
			{
				mat.baseColorFactor = Vector4(
					static_cast<float>(gltfMat.pbrMetallicRoughness.baseColorFactor[0]),
					static_cast<float>(gltfMat.pbrMetallicRoughness.baseColorFactor[1]),
					static_cast<float>(gltfMat.pbrMetallicRoughness.baseColorFactor[2]),
					static_cast<float>(gltfMat.pbrMetallicRoughness.baseColorFactor[3])
				);
			}

			if (gltfMat.pbrMetallicRoughness.baseColorTexture.index >= 0)
			{
				const int texIndex = gltfMat.pbrMetallicRoughness.baseColorTexture.index;
				const int sourceIndex = gltfModel.textures[texIndex].source;
				if (sourceIndex >= 0 && static_cast<size_t>(sourceIndex) < gltfModel.images.size() && !gltfModel.images[sourceIndex].uri.empty())
					mat.diffuseTexturePath = (filePath.parent_path() / gltfModel.images[sourceIndex].uri).string();
			}

			if (gltfMat.normalTexture.index >= 0)
			{
				const int texIndex = gltfMat.normalTexture.index;
				const int sourceIndex = gltfModel.textures[texIndex].source;
				if (sourceIndex >= 0 && static_cast<size_t>(sourceIndex) < gltfModel.images.size() && !gltfModel.images[sourceIndex].uri.empty())
					mat.normalTexturePath = (filePath.parent_path() / gltfModel.images[sourceIndex].uri).string();
			}

			if (gltfMat.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0)
			{
				const int texIndex = gltfMat.pbrMetallicRoughness.metallicRoughnessTexture.index;
				const int sourceIndex = gltfModel.textures[texIndex].source;
				if (sourceIndex >= 0 && static_cast<size_t>(sourceIndex) < gltfModel.images.size() && !gltfModel.images[sourceIndex].uri.empty())
					mat.metallicRoughnessTexturePath = (filePath.parent_path() / gltfModel.images[sourceIndex].uri).string();
			}

			// Foliage exported as BLEND (e.g. Sketchfab trees) renders far cleaner as alpha-tested cutout in deferred + RT shadows
			if (mat.alphaMode == EImportedAlphaMode::Blend &&
				(IsLikelyFoliageName(gltfMat.name) || (mat.diffuseTexturePath.has_value() && IsLikelyFoliageName(mat.diffuseTexturePath.value()))))
			{
				mat.alphaMode = EImportedAlphaMode::Mask;
				mat.alphaCutoff = 0.45f;
			}

			importedModel.materials.push_back(std::move(mat));
		}

		// Extract raw primitives per glTF mesh
		std::vector<ImportedMesh>			 rawPrimitives;
		std::vector<std::vector<ImportedId>> meshPrimitiveMap(gltfModel.meshes.size());

		for (size_t gltfMeshIdx = 0; gltfMeshIdx < gltfModel.meshes.size(); ++gltfMeshIdx)
		{
			const auto& gltfMesh = gltfModel.meshes[gltfMeshIdx];
			for (const auto& primitive : gltfMesh.primitives)
			{
				if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
					continue;

				ImportedMesh currentMesh{};
				currentMesh.id = static_cast<uint32_t>(rawPrimitives.size());

				if (primitive.material >= 0)
					currentMesh.materialId = static_cast<ImportedId>(primitive.material);

				if (primitive.indices >= 0)
				{
					const tinygltf::Accessor&	indexAccessor = gltfModel.accessors[primitive.indices];
					const tinygltf::BufferView& indexBufferView = gltfModel.bufferViews[indexAccessor.bufferView];
					const tinygltf::Buffer&		indexBuffer = gltfModel.buffers[indexBufferView.buffer];

					const uint8_t* dataPtr = indexBuffer.data.data() + indexBufferView.byteOffset + indexAccessor.byteOffset;
					currentMesh.indices.reserve(indexAccessor.count);
					for (size_t i = 0; i < indexAccessor.count; ++i)
					{
						uint32_t index = 0;
						if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
							index = dataPtr[i];
						else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
							index = reinterpret_cast<const uint16_t*>(dataPtr)[i];
						else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
							index = reinterpret_cast<const uint32_t*>(dataPtr)[i];
						currentMesh.indices.push_back(index);
					}
				}

				if (primitive.attributes.contains("POSITION"))
				{
					const tinygltf::Accessor&	posAccessor = gltfModel.accessors[primitive.attributes.at("POSITION")];
					const tinygltf::BufferView& posBufferView = gltfModel.bufferViews[posAccessor.bufferView];
					const tinygltf::Buffer&		posBuffer = gltfModel.buffers[posBufferView.buffer];

					const uint8_t* posPtr = posBuffer.data.data() + posBufferView.byteOffset + posAccessor.byteOffset;
					const size_t   stride = posBufferView.byteStride == 0 ? sizeof(float) * 3 : posBufferView.byteStride;

					currentMesh.vertices.reserve(posAccessor.count);
					for (size_t i = 0; i < posAccessor.count; ++i)
					{
						const auto* position = reinterpret_cast<const float*>(posPtr + (i * stride));

						VertexData vertex{};
						if (input.preTransform)
							vertex.position = {position[0], position[1], position[2]};
						else
							vertex.position = {position[0], position[2], -position[1]};

						vertex.color = {1.0f, 1.0f, 1.0f};
						vertex.normal = {0.0f, 1.0f, 0.0f};
						currentMesh.vertices.push_back(vertex);
					}
				}

				if (currentMesh.vertices.empty())
					continue;

				if (primitive.attributes.contains("NORMAL"))
				{
					const tinygltf::Accessor&	normAccessor = gltfModel.accessors[primitive.attributes.at("NORMAL")];
					const tinygltf::BufferView& normBufferView = gltfModel.bufferViews[normAccessor.bufferView];
					const tinygltf::Buffer&		normBuffer = gltfModel.buffers[normBufferView.buffer];

					const uint8_t* normPtr = normBuffer.data.data() + normBufferView.byteOffset + normAccessor.byteOffset;
					const size_t   stride = normBufferView.byteStride == 0 ? sizeof(float) * 3 : normBufferView.byteStride;

					const size_t count = std::min(normAccessor.count, currentMesh.vertices.size());
					for (size_t i = 0; i < count; ++i)
					{
						const auto* normal = reinterpret_cast<const float*>(normPtr + (i * stride));
						if (input.preTransform)
							currentMesh.vertices[i].normal = {normal[0], normal[1], normal[2]};
						else
							currentMesh.vertices[i].normal = {normal[0], normal[2], -normal[1]};
					}
				}

				if (primitive.attributes.contains("TEXCOORD_0"))
				{
					const tinygltf::Accessor&	texAccessor = gltfModel.accessors[primitive.attributes.at("TEXCOORD_0")];
					const tinygltf::BufferView& texBufferView = gltfModel.bufferViews[texAccessor.bufferView];
					const tinygltf::Buffer&		texBuffer = gltfModel.buffers[texBufferView.buffer];

					const uint8_t* texPtr = texBuffer.data.data() + texBufferView.byteOffset + texAccessor.byteOffset;
					const size_t   stride = texBufferView.byteStride == 0 ? sizeof(float) * 2 : texBufferView.byteStride;

					const size_t count = std::min(texAccessor.count, currentMesh.vertices.size());
					for (size_t i = 0; i < count; ++i)
					{
						const auto* texCoord = reinterpret_cast<const float*>(texPtr + (i * stride));
						currentMesh.vertices[i].uv = {texCoord[0], texCoord[1]};
					}
				}

				if (currentMesh.indices.empty())
				{
					currentMesh.indices.resize(currentMesh.vertices.size());
					for (uint32_t i = 0; i < static_cast<uint32_t>(currentMesh.vertices.size()); ++i)
						currentMesh.indices[i] = i;
				}

				meshPrimitiveMap[gltfMeshIdx].push_back(currentMesh.id);
				rawPrimitives.push_back(std::move(currentMesh));
			}
		}

		// Parse node hierarchy
		std::vector<ImportedNode> parsedNodes;
		parsedNodes.reserve(gltfModel.nodes.size());
		for (const auto& gltfNode : gltfModel.nodes)
		{
			ImportedNode node{};
			node.name = gltfNode.name;
			node.transform = Matrix4::Identity;

			if (gltfNode.matrix.size() == 16)
			{
				float matData[16];
				for (int i = 0; i < 16; ++i)
					matData[i] = static_cast<float>(gltfNode.matrix[i]);

				node.transform = Matrix4::FromColumnMajor(matData);
			}
			else
			{
				Vector3 translation = Vector3::Zero;
				if (gltfNode.translation.size() == 3)
				{
					translation = Vector3{
						static_cast<float>(gltfNode.translation[0]),
						static_cast<float>(gltfNode.translation[1]),
						static_cast<float>(gltfNode.translation[2])
					};
				}

				Vector3 scale = Vector3::One;
				if (gltfNode.scale.size() == 3)
				{
					scale = Vector3{
						static_cast<float>(gltfNode.scale[0]),
						static_cast<float>(gltfNode.scale[1]),
						static_cast<float>(gltfNode.scale[2])
					};
				}

				Quaternion rotation = Quaternion::Identity;
				if (gltfNode.rotation.size() == 4)
				{
					rotation = Quaternion{
						static_cast<float>(gltfNode.rotation[3]),
						static_cast<float>(gltfNode.rotation[0]),
						static_cast<float>(gltfNode.rotation[1]),
						static_cast<float>(gltfNode.rotation[2])
					}.Normalized();
				}

				node.transform = Matrix4::TRS(translation, rotation, scale);
			}

			if (gltfNode.mesh >= 0 && static_cast<size_t>(gltfNode.mesh) < meshPrimitiveMap.size())
				node.meshes = meshPrimitiveMap[gltfNode.mesh];

			node.children.reserve(gltfNode.children.size());
			for (const int childIdx : gltfNode.children)
			{
				if (childIdx >= 0)
					node.children.push_back(static_cast<ImportedId>(childIdx));
			}

			parsedNodes.push_back(std::move(node));
		}

		std::vector<ImportedId> rootNodeIndices;
		if (!gltfModel.scenes.empty())
		{
			const int sceneIdx = gltfModel.defaultScene >= 0 ? gltfModel.defaultScene : 0;
			for (const int rootNodeIdx : gltfModel.scenes[sceneIdx].nodes)
			{
				if (rootNodeIdx >= 0)
					rootNodeIndices.push_back(static_cast<ImportedId>(rootNodeIdx));
			}
		}
		else
		{
			for (size_t i = 0; i < parsedNodes.size(); ++i)
				rootNodeIndices.push_back(static_cast<ImportedId>(i));
		}

		if (input.preTransform && !parsedNodes.empty())
		{
			// Bake world transforms and merge meshes by material ID (collapses 500+ foliage nodes into 1 mesh per material)
			std::map<int64_t, ImportedMesh> mergedByMaterial;

			std::function<void(ImportedId, const Matrix4&)> bakeNode = [&](const ImportedId nodeIdx, const Matrix4& parentMat) {
				if (nodeIdx >= parsedNodes.size())
					return;

				const auto&		node = parsedNodes[nodeIdx];
				const Matrix4	worldMat = parentMat * node.transform;
				const glm::mat4 worldGlm = worldMat.Value();
				const glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(worldGlm)));
				const bool		flipWinding = glm::determinant(glm::mat3(worldGlm)) < 0.0f;

				for (const ImportedId primId : node.meshes)
				{
					const auto&	  src = rawPrimitives[primId];
					const int64_t matKey = src.materialId.has_value() ? static_cast<int64_t>(src.materialId.value()) : -1;
					auto&		  dst = mergedByMaterial[matKey];
					if (src.materialId.has_value())
						dst.materialId = src.materialId;

					const uint32_t baseVertex = static_cast<uint32_t>(dst.vertices.size());
					dst.vertices.reserve(dst.vertices.size() + src.vertices.size());
					for (const auto& v : src.vertices)
					{
						VertexData tv = v;
						const glm::vec4 p = worldGlm * glm::vec4(v.position.Value(), 1.0f);
						glm::vec3		n = normalMat * v.normal.Value();
						const float		nLen = glm::length(n);
						n = (nLen > 1e-6f) ? (n / nLen) : glm::vec3(0.0f, 1.0f, 0.0f);
						tv.position = Vector3(p.x, p.y, p.z);
						tv.normal = Vector3(n);
						dst.vertices.push_back(tv);
					}

					dst.indices.reserve(dst.indices.size() + src.indices.size());
					if (flipWinding)
					{
						for (size_t i = 0; i + 2 < src.indices.size(); i += 3)
						{
							dst.indices.push_back(baseVertex + src.indices[i]);
							dst.indices.push_back(baseVertex + src.indices[i + 2]);
							dst.indices.push_back(baseVertex + src.indices[i + 1]);
						}
					}
					else
					{
						for (const uint32_t idx : src.indices)
							dst.indices.push_back(baseVertex + idx);
					}
				}

				for (const ImportedId childIdx : node.children)
					bakeNode(childIdx, worldMat);
			};

			for (const ImportedId rootIdx : rootNodeIndices)
				bakeNode(rootIdx, Matrix4::Identity);

			ImportedNode bakedRoot{};
			bakedRoot.name = filePath.stem().string();
			bakedRoot.transform = Matrix4::Identity;

			for (auto& [matKey, merged] : mergedByMaterial)
			{
				if (merged.vertices.empty() || merged.indices.empty())
					continue;
				merged.id = static_cast<ImportedId>(importedModel.meshes.size());
				OptimizeAndBoundMesh(merged);
				bakedRoot.meshes.push_back(merged.id);
				importedModel.meshes.push_back(std::move(merged));
			}

			importedModel.nodes.push_back(std::move(bakedRoot));
			importedModel.rootNodes.push_back(0);
			return importedModel;
		}

		// Non-preTransform path: preserve node hierarchy
		for (auto& prim : rawPrimitives)
			OptimizeAndBoundMesh(prim);
		importedModel.meshes = std::move(rawPrimitives);
		importedModel.nodes = std::move(parsedNodes);
		importedModel.rootNodes = std::move(rootNodeIndices);
		return importedModel;
	}
}
