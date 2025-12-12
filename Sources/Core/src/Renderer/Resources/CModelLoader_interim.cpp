#include "BetterThanNothing.hpp"
#include "CModelLoader_interim.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace BetterThanNothing
{
	ModelData CModelLoader_interim::LoadModel(const std::string &path)
	{
		ModelData model;

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn;
		std::string err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
			throw std::runtime_error(err);

		if (!warn.empty())
			LOG_WARNING(warn);

		for (const auto& shape : shapes) {
			MeshData mesh;
			std::unordered_map<VertexData, uint32_t, VertexHash, VertexEqual> uniqueVertices{};

			for (const auto& index : shape.mesh.indices) {
				VertexData vertex{};

				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.color = {1.0f, 1.0f, 1.0f};

				if (!attrib.texcoords.empty() && index.texcoord_index != -1) {
					vertex.uv = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
					};
				} else {
					vertex.uv = {0.0f, 0.0f};
				}

				if (!uniqueVertices.contains(vertex)) {
					uniqueVertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
					mesh.vertices.push_back(vertex);
				}

				mesh.indices.push_back(uniqueVertices[vertex]);
			}
			model.meshes.push_back(mesh);
		}

		return model;
	}
}
