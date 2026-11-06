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
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE

#include <tiny_gltf.h>

#ifdef ERROR
#undef ERROR
#endif

#include <Common/Logger.hpp>
#include <ResourceImporter/Importer.hpp>
#include <ResourceImporter/ResourceImporter_Types.hpp>
#include <cstring>
#include <filesystem>
#include <meshoptimizer.h>
#include <unordered_map>
#include <unordered_set>

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
		return EImportedAlphaMode::Opaque;
	}

	ImportedModel GltfImporter::Import(const ModelImportInfo& input)
	{
		ImportedModel importedModel;
		importedModel.path = input.path;

		tinygltf::Model	   gltfModel;
		tinygltf::TinyGLTF loader;
		std::string		   err;
		std::string		   warn;

		std::filesystem::path filePath(input.path);
		bool ret = false;
		if (filePath.extension() == ".glb")
		{
			ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, input.path);
		}
		else
		{
			ret = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, input.path);
		}

		if (!warn.empty())
			Logger::Log<WARNING>("TinyGLTF warning: {}", warn);

		if (!err.empty())
			Logger::Log<ERROR>("TinyGLTF error: {}", err);

		if (!ret)
			throw std::runtime_error("Failed to parse glTF file: " + input.path + " - " + err);

		for (size_t matIdx = 0; matIdx < gltfModel.materials.size(); ++matIdx)
		{
			const auto& gltfMat = gltfModel.materials[matIdx];
			ImportedMaterial mat{};
			mat.id = static_cast<uint32_t>(matIdx);
			mat.name = gltfMat.name.empty() ? ("Material_" + std::to_string(matIdx)) : gltfMat.name;
			mat.alphaMode = Internal_RetrieveAlphaMode(gltfMat.alphaMode);
			mat.alphaCutoff = static_cast<float>(gltfMat.alphaCutoff);

			// Base Color Factor & Texture
			const auto& pbr = gltfMat.pbrMetallicRoughness;
			if (pbr.baseColorFactor.size() == 4)
			{
				mat.baseColorFactor = Vector4(
					static_cast<float>(pbr.baseColorFactor[0]),
					static_cast<float>(pbr.baseColorFactor[1]),
					static_cast<float>(pbr.baseColorFactor[2]),
					static_cast<float>(pbr.baseColorFactor[3])
				);
			}
			mat.metallicFactor = static_cast<float>(pbr.metallicFactor);
			mat.roughnessFactor = static_cast<float>(pbr.roughnessFactor);

			if (gltfMat.emissiveFactor.size() == 3)
			{
				mat.emissiveFactor = Vector3(
					static_cast<float>(gltfMat.emissiveFactor[0]),
					static_cast<float>(gltfMat.emissiveFactor[1]),
					static_cast<float>(gltfMat.emissiveFactor[2])
				);
			}

			auto getTexturePath = [&](int textureIndex) -> std::optional<std::string> {
				if (textureIndex < 0 || textureIndex >= static_cast<int>(gltfModel.textures.size()))
					return std::nullopt;
				int sourceImage = gltfModel.textures[textureIndex].source;
				if (sourceImage < 0 || sourceImage >= static_cast<int>(gltfModel.images.size()))
					return std::nullopt;
				const auto& image = gltfModel.images[sourceImage];
				if (image.uri.empty())
					return std::nullopt;
				return (filePath.parent_path() / image.uri).string();
			};

			mat.diffuseTexturePath = getTexturePath(pbr.baseColorTexture.index);
			mat.normalTexturePath = getTexturePath(gltfMat.normalTexture.index);
			mat.metallicRoughnessTexturePath = getTexturePath(pbr.metallicRoughnessTexture.index);
			mat.occlusionTexturePath = getTexturePath(gltfMat.occlusionTexture.index);
			mat.emissiveTexturePath = getTexturePath(gltfMat.emissiveTexture.index);

			importedModel.materials.push_back(mat);
		}

		std::unordered_map<int, std::vector<ImportedId>> gltfMeshToPrimitives;

		for (size_t meshIdx = 0; meshIdx < gltfModel.meshes.size(); ++meshIdx)
		{
			const auto& gltfMesh = gltfModel.meshes[meshIdx];

			for (const auto& primitive : gltfMesh.primitives)
			{
				if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
					continue;

				ImportedMesh currentMesh{};
				currentMesh.id = static_cast<uint32_t>(importedModel.meshes.size());
				currentMesh.name = gltfMesh.name.empty() ? ("Mesh_" + std::to_string(currentMesh.id)) : gltfMesh.name;

				if (primitive.material >= 0)
					currentMesh.materialId = static_cast<ImportedId>(primitive.material);

				if (primitive.indices >= 0)
				{
					const tinygltf::Accessor&	indexAccessor = gltfModel.accessors[primitive.indices];
					const tinygltf::BufferView& indexBufferView = gltfModel.bufferViews[indexAccessor.bufferView];
					const tinygltf::Buffer&		indexBuffer = gltfModel.buffers[indexBufferView.buffer];

					const uint8_t* dataPtr = indexBuffer.data.data() + indexBufferView.byteOffset + indexAccessor.byteOffset;
					for (size_t i = 0; i < indexAccessor.count; ++i)
					{
						uint32_t index = 0;
						if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
						{
							index = reinterpret_cast<const uint8_t*>(dataPtr)[i];
						}
						else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
						{
							index = reinterpret_cast<const uint16_t*>(dataPtr)[i];
						}
						else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
						{
							index = reinterpret_cast<const uint32_t*>(dataPtr)[i];
						}
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
						auto position = reinterpret_cast<const float*>(posPtr + (i * stride));
						VertexData vertex{};
						vertex.position = {position[0], position[1], position[2]};
						vertex.color = {1.0f, 1.0f, 1.0f};
						vertex.tangent = {1.0f, 0.0f, 0.0f, 1.0f};
						currentMesh.vertices.push_back(vertex);
					}
				}

				if (primitive.attributes.contains("NORMAL"))
				{
					const tinygltf::Accessor&	normAccessor = gltfModel.accessors[primitive.attributes.at("NORMAL")];
					const tinygltf::BufferView& normBufferView = gltfModel.bufferViews[normAccessor.bufferView];
					const tinygltf::Buffer&		normBuffer = gltfModel.buffers[normBufferView.buffer];

					const uint8_t* normPtr = normBuffer.data.data() + normBufferView.byteOffset + normAccessor.byteOffset;
					const size_t   stride = normBufferView.byteStride == 0 ? sizeof(float) * 3 : normBufferView.byteStride;
					const size_t   count = std::min(normAccessor.count, currentMesh.vertices.size());

					for (size_t i = 0; i < count; ++i)
					{
						auto normal = reinterpret_cast<const float*>(normPtr + (i * stride));
						currentMesh.vertices[i].normal = {normal[0], normal[1], normal[2]};
					}
				}

				if (primitive.attributes.contains("TEXCOORD_0"))
				{
					const tinygltf::Accessor&	texAccessor = gltfModel.accessors[primitive.attributes.at("TEXCOORD_0")];
					const tinygltf::BufferView& texBufferView = gltfModel.bufferViews[texAccessor.bufferView];
					const tinygltf::Buffer&		texBuffer = gltfModel.buffers[texBufferView.buffer];

					const uint8_t* texPtr = texBuffer.data.data() + texBufferView.byteOffset + texAccessor.byteOffset;
					const size_t   stride = texBufferView.byteStride == 0 ? sizeof(float) * 2 : texBufferView.byteStride;
					const size_t   count = std::min(texAccessor.count, currentMesh.vertices.size());

					for (size_t i = 0; i < count; ++i)
					{
						auto texCoord = reinterpret_cast<const float*>(texPtr + (i * stride));
						currentMesh.vertices[i].uv = {texCoord[0], texCoord[1]};
					}
				}

				if (primitive.attributes.contains("TANGENT"))
				{
					const tinygltf::Accessor&	tangAccessor = gltfModel.accessors[primitive.attributes.at("TANGENT")];
					const tinygltf::BufferView& tangBufferView = gltfModel.bufferViews[tangAccessor.bufferView];
					const tinygltf::Buffer&		tangBuffer = gltfModel.buffers[tangBufferView.buffer];

					const uint8_t* tangPtr = tangBuffer.data.data() + tangBufferView.byteOffset + tangAccessor.byteOffset;
					const size_t   stride = tangBufferView.byteStride == 0 ? sizeof(float) * 4 : tangBufferView.byteStride;
					const size_t   count = std::min(tangAccessor.count, currentMesh.vertices.size());

					for (size_t i = 0; i < count; ++i)
					{
						auto tangent = reinterpret_cast<const float*>(tangPtr + (i * stride));
						currentMesh.vertices[i].tangent = Vector4(tangent[0], tangent[1], tangent[2], tangent[3]);
					}
				}

				bool isIndexed = !currentMesh.indices.empty();
				const size_t vertex_count_in = currentMesh.vertices.size();
				const size_t index_count_in = isIndexed ? currentMesh.indices.size() : vertex_count_in;

				std::vector<unsigned int> remap(vertex_count_in);

				// clang-format off
				const size_t unique_vertex_count = meshopt_generateVertexRemap(
					remap.data(),
					isIndexed ? currentMesh.indices.data() : nullptr,
					index_count_in,
					currentMesh.vertices.data(),
					vertex_count_in,
					sizeof(VertexData)
				);
				// clang-format on

				std::vector<VertexData> new_vertices(unique_vertex_count);
				std::vector<uint32_t>	new_indices(index_count_in);

				meshopt_remapVertexBuffer(new_vertices.data(), currentMesh.vertices.data(), vertex_count_in, sizeof(VertexData), remap.data());
				meshopt_remapIndexBuffer(new_indices.data(), isIndexed ? currentMesh.indices.data() : nullptr, index_count_in, remap.data());

				meshopt_optimizeVertexCache(new_indices.data(), new_indices.data(), index_count_in, unique_vertex_count);
				meshopt_optimizeOverdraw(new_indices.data(), new_indices.data(), index_count_in, &new_vertices[0].position.x(), unique_vertex_count, sizeof(VertexData), 1.05f);
				meshopt_optimizeVertexFetch(new_vertices.data(), new_indices.data(), index_count_in, new_vertices.data(), unique_vertex_count, sizeof(VertexData));

				currentMesh.vertices = new_vertices;
				currentMesh.indices = new_indices;

				gltfMeshToPrimitives[static_cast<int>(meshIdx)].push_back(currentMesh.id);
				importedModel.meshes.push_back(currentMesh);
			}
		}

		std::unordered_set<int> childNodeIndices;

		for (size_t nodeIdx = 0; nodeIdx < gltfModel.nodes.size(); ++nodeIdx)
		{
			const auto& gltfNode = gltfModel.nodes[nodeIdx];
			ImportedNode node{};
			node.name = gltfNode.name.empty() ? ("Node_" + std::to_string(nodeIdx)) : gltfNode.name;
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
				Vector3 translation{0.0f, 0.0f, 0.0f};
				if (gltfNode.translation.size() == 3)
				{
					translation = Vector3{
						static_cast<float>(gltfNode.translation[0]), static_cast<float>(gltfNode.translation[1]),
						static_cast<float>(gltfNode.translation[2])
					};
				}

				Vector3 scale{1.0f, 1.0f, 1.0f};
				if (gltfNode.scale.size() == 3)
				{
					scale = Vector3{
						static_cast<float>(gltfNode.scale[0]), static_cast<float>(gltfNode.scale[1]),
						static_cast<float>(gltfNode.scale[2])
					};
				}

				Quaternion rotation = Quaternion::Identity;
				if (gltfNode.rotation.size() == 4)
				{
					// glTF specifies rotation as [x, y, z, w]; Quaternion constructor expects (w, x, y, z)
					rotation = Quaternion(
						static_cast<float>(gltfNode.rotation[3]), // w
						static_cast<float>(gltfNode.rotation[0]), // x
						static_cast<float>(gltfNode.rotation[1]), // y
						static_cast<float>(gltfNode.rotation[2])  // z
					);
				}

				node.transform = Matrix4::Identity.Translate(translation).Rotate(rotation).Scale(scale);
			}

			if (gltfNode.mesh >= 0 && gltfMeshToPrimitives.contains(gltfNode.mesh))
			{
				node.meshes = gltfMeshToPrimitives[gltfNode.mesh];
			}

			for (int childIdx : gltfNode.children)
			{
				if (childIdx >= 0)
				{
					node.children.push_back(static_cast<uint32_t>(childIdx));
					childNodeIndices.insert(childIdx);
				}
			}

			importedModel.children.push_back(node);
		}

		for (size_t i = 0; i < importedModel.children.size(); ++i)
		{
			if (!childNodeIndices.contains(static_cast<int>(i)))
				importedModel.rootNodes.push_back(static_cast<uint32_t>(i));
		}

		return importedModel;
	};
}
