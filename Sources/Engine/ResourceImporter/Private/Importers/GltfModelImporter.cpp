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

#include "GltfModelImporter.hpp"
#include <tiny_gltf.h>

#include <ResourceImporter/ResourceImporter_Types.hpp>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <meshoptimizer.h>

namespace BetterThanNothing
{
	static EImportedAlphaMode Internal_RetrieveAlphaMode(const std::string& value)
	{
		if (value == "OPAQUE")
			return EImportedAlphaMode::Opaque;
		if (value == "MASK")
			return EImportedAlphaMode::Mask;
		if (value == "BLEND")
			return EImportedAlphaMode::Blend;
		throw std::runtime_error("Unknown Alpha Mode value: " + value);
	}

	ImportedModel GltfModelImporter::Import(const GltfModelImportInfo& input)
	{
		ImportedModel importedModel;
		importedModel.path = input.path;

		tinygltf::Model	   gltfModel;
		tinygltf::TinyGLTF loader;
		std::string		   err;
		std::string		   warn;

		std::filesystem::path filePath(input.path);
		bool				  ret = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, input.path);

		if (!warn.empty())
			throw std::runtime_error("TinyGLTF warning: " + warn);

		if (!err.empty())
			throw std::runtime_error("TinyGLTF error: " + err);

		if (!ret)
			throw std::runtime_error("Failed to parse glTF file: " + input.path);

		for (const auto& gltfMat : gltfModel.materials)
		{
			ImportedMaterial mat{};

			mat.alphaMode = Internal_RetrieveAlphaMode(gltfMat.alphaMode);
			mat.alphaCutoff = gltfMat.alphaCutoff;

			if (gltfMat.pbrMetallicRoughness.baseColorTexture.index >= 0)
			{
				int	  texIndex = gltfMat.pbrMetallicRoughness.baseColorTexture.index;
				auto& image = gltfModel.images[gltfModel.textures[texIndex].source];
				mat.diffuseTexturePath = (filePath.parent_path() / image.uri).string();
			}

			importedModel.materials.push_back(mat);
		}

		for (const auto& gltfMesh : gltfModel.meshes)
		{
			for (const auto& primitive : gltfMesh.primitives)
			{
				if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
					continue;

				ImportedMesh currentMesh{};
				currentMesh.id = importedModel.meshes.size();

				if (primitive.material >= 0)
					currentMesh.materialId = primitive.material;

				if (primitive.indices >= 0)
				{
					const tinygltf::Accessor&	indexAccessor = gltfModel.accessors[primitive.indices];
					const tinygltf::BufferView& indexBufferView = gltfModel.bufferViews[indexAccessor.bufferView];
					const tinygltf::Buffer&		indexBuffer = gltfModel.buffers[indexBufferView.buffer];

					const uint8_t* dataPtr = indexBuffer.data.data() + indexBufferView.byteOffset + indexAccessor.byteOffset;
					for (size_t i = 0; i < indexAccessor.count; ++i)
					{
						uint32_t index = 0;
						if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
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

					for (size_t i = 0; i < posAccessor.count; ++i)
					{
						auto position = reinterpret_cast<const float*>(posPtr + (i * stride));

						VertexData vertex{};

						if (input.preTransform)
						{
							vertex.position = {position[0], position[1], position[2]};
						}
						else
						{
							vertex.position = {position[0], position[2], -position[1]};
						}

						vertex.color = {1.0f, 1.0f, 1.0f};

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

					for (size_t i = 0; i < normAccessor.count; ++i)
					{
						auto normal = reinterpret_cast<const float*>(normPtr + (i * stride));

						if (input.preTransform)
						{
							currentMesh.vertices[i].normal = {normal[0], normal[1], normal[2]};
						}
						else
						{
							currentMesh.vertices[i].normal = {normal[0], normal[2], -normal[1]};
						}
					}
				}

				if (primitive.attributes.contains("TEXCOORD_0"))
				{
					const tinygltf::Accessor&	texAccessor = gltfModel.accessors[primitive.attributes.at("TEXCOORD_0")];
					const tinygltf::BufferView& texBufferView = gltfModel.bufferViews[texAccessor.bufferView];
					const tinygltf::Buffer&		texBuffer = gltfModel.buffers[texBufferView.buffer];

					const uint8_t* texPtr = texBuffer.data.data() + texBufferView.byteOffset + texAccessor.byteOffset; // Renommé normPtr en texPtr
					const size_t stride = texBufferView.byteStride == 0 ? sizeof(float) * 2 : texBufferView.byteStride;

					for (size_t i = 0; i < texAccessor.count; ++i)
					{
						auto texCoord = reinterpret_cast<const float*>(texPtr + (i * stride));
						currentMesh.vertices[i].uv = {texCoord[0], texCoord[1]};
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

				importedModel.meshes.push_back(currentMesh);
			}
		}

		for (const auto& gltfNode : gltfModel.nodes)
		{
			ImportedNode node{};
			node.transform = Matrix4::Identity;

			if (gltfNode.matrix.size() == 16)
			{
				float matData[16];
				for (int i = 0; i < 16; ++i)
					matData[i] = static_cast<float>(gltfNode.matrix[i]);

				node.transform = Matrix4(matData);
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
					scale =
						Vector3{static_cast<float>(gltfNode.scale[0]), static_cast<float>(gltfNode.scale[1]), static_cast<float>(gltfNode.scale[2])};
				}

				Quaternion rotation = Quaternion::Identity;
				if (gltfNode.rotation.size() == 4)
				{
					rotation = Quaternion{
						static_cast<float>(gltfNode.rotation[0]), // x
						static_cast<float>(gltfNode.rotation[1]), // y
						static_cast<float>(gltfNode.rotation[2]), // z
						static_cast<float>(gltfNode.rotation[3])  // w
					};
				}

				node.transform = Matrix4::Identity.Translate(translation).Rotate(rotation).Scale(scale);
			}

			if (gltfNode.mesh >= 0)
				node.mesh = gltfNode.mesh;

			importedModel.children.push_back(node);
		}

		return importedModel;
	};
}
