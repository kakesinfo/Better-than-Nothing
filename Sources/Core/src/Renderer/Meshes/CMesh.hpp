//
// Created by acoezard on 29/11/2025.
//

#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class CVulkanBuffer;

	class CMesh
	{
	private:
		std::vector<CVertex> m_vertices;
		std::vector<uint16> m_indices;

		CVulkanBuffer m_vertexBuffer;
		CVulkanBuffer m_indexBuffer;

		bool m_bIsBuffered = false;
	public:
		explicit CMesh() = default;
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

		void Build()
		{
			if (m_bIsBuffered)
				throw std::runtime_error("Mesh is already built and buffered.");

			m_bIsBuffered = true;

			const uint32 vertexBufferSize = sizeof(m_vertices[0]) * m_vertices.size();
			const void* vertexBufferData = m_vertices.data();

			m_vertexBuffer = CVulkanBufferFactory::CreateVertexBuffer(vertexBufferData, vertexBufferSize);

			const uint32 indexBufferSize = sizeof(m_indices[0]) * m_indices.size();
			const void* indexBufferData = m_indices.data();

			m_indexBuffer = CVulkanBufferFactory::CreateIndexBuffer(indexBufferData, indexBufferSize);
		}

		CVulkanBuffer& GetVertexBuffer()
		{
			if (!m_bIsBuffered)
				throw std::runtime_error("Mesh is not yet built and buffered.");
			return m_vertexBuffer;
		}

		CVulkanBuffer& GetIndexBuffer()
		{
			if (!m_bIsBuffered)
				throw std::runtime_error("Mesh is not yet built and buffered.");
			return m_indexBuffer;
		}

		std::vector<CVertex>& GetVertices()
		{
			return m_vertices;
		}

		std::vector<uint16>& GetIndices()
		{
			return m_indices;
		}
	};
}