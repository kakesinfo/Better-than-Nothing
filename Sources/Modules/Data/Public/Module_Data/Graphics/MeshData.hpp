#pragma once

#include "VertexData.hpp"
#include <vector>

namespace BetterThanNothing
{
	struct MeshData
	{
		std::vector<VertexData> vertices;
		std::vector<std::uint32_t> indices;

		std::uint32_t materialId;
		std::string texturePath;

		std::uint64_t id = 0; // TODO: move to loader
		explicit MeshData() : materialId(-1), id(s_globalIdCounter++)
		{
		}

		// TODO: put all these next lines in a builder

		void AddVertex(const float x, const float y, const float z, const float r, const float g, const float b, const float u, const float v, const float nx, const float ny, const float nz)
		{
			AddVertex(glm::vec3(x, y, z), glm::vec3(r, g, b), glm::vec2(u, v), glm::vec3(nx, ny, nz));
		}

		void AddVertex(const glm::vec3 position, const glm::vec3 color, const glm::vec2 uv, const glm::vec3 normal)
		{
			AddVertex(VertexData{position, color, uv, normal});
		}

		void AddVertex(const VertexData &vertex)
		{
			vertices.push_back(vertex);
		}

		void AddIndices(const std::vector<std::uint32_t> &newIndices)
		{
			indices.insert(indices.end(), newIndices.begin(), newIndices.end());
		}

		void AddIndex(const std::uint32_t index)
		{
			indices.push_back(index);
		}

	private:
		inline static std::uint64_t s_globalIdCounter = 0;
	};
}