//
// Created by acoezard on 29/11/2025.
//

#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	struct MeshData
	{
		std::vector<VertexData> vertices;
		std::vector<uint16> indices;
		uint64 id = 0;

		explicit MeshData(): id(s_globalIdCounter++) {}

		// TODO: put all these next lines in a builder

		void AddVertex(const float x, const float y, const float z, const float r, const float g, const float b, const float u, const float v)
		{
			AddVertex(glm::vec3(x, y, z), glm::vec3(r, g, b), glm::vec2(u, v));
		}

		void AddVertex(const glm::vec3 position, const glm::vec3 color, const glm::vec2 uv)
		{
			AddVertex(VertexData{position, color, uv});
		}

		void AddVertex(const VertexData &vertex)
		{
			vertices.push_back(vertex);
		}

		void AddIndices(const std::vector<uint16> &newIndices)
		{
			indices.insert(indices.end(), newIndices.begin(), newIndices.end());
		}

		void AddIndex(const uint16 index)
		{
			indices.push_back(index);
		}

	private:
		inline static uint64 s_globalIdCounter = 0;
	};
}