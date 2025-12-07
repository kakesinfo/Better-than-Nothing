//
// Created by acoezard on 29/11/2025.
//

#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class CMesh
	{
	public:
		explicit CMesh(): m_id(s_globalIdCounter++) {}
		~CMesh() = default;

		void AddVertex(const float x, const float y, const float r, const float g, const float b)
		{
			AddVertex(glm::vec2(x, y), glm::vec3(r, g, b));
		}

		void AddVertex(const glm::vec2 position, const glm::vec3 color)
		{
			AddVertex(CVertex{position, color});
		}

		void AddVertex(const CVertex &vertex)
		{
			m_vertices.push_back(vertex);
		}

		void AddIndices(const std::vector<uint16> &newIndices)
		{
			m_indices.insert(m_indices.end(), newIndices.begin(), newIndices.end());
		}

		void AddIndex(const uint16 index)
		{
			m_indices.push_back(index);
		}

		std::vector<CVertex>& GetVertices()
		{
			return m_vertices;
		}

		std::vector<uint16>& GetIndices()
		{
			return m_indices;
		}

		[[nodiscard]]
		uint64 GetId() const { return m_id; }

	private:
		std::vector<CVertex> m_vertices;
		std::vector<uint16> m_indices;

		inline static uint64 s_globalIdCounter = 0;
		uint64 m_id = 0;
	};
}