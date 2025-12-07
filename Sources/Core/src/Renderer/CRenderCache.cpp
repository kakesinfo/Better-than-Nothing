#include "CRenderCache.hpp"

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	MeshGpuData* CRenderCache::GetOrCreateMeshGpuData(CMesh* pMesh)
	{
		auto it = m_meshCache.find(pMesh->GetId());
		if (it != m_meshCache.end())
			return it->second.get();

		auto meshGpuData = std::make_unique<MeshGpuData>();

		const std::vector<CVertex> vertices = pMesh->GetVertices();
		const std::vector<uint16> indices = pMesh->GetIndices();

		if (vertices.empty() || indices.empty())
			throw std::runtime_error("Mesh has no vertices or indices!");

		const uint32 vertexBufferSize = sizeof(vertices[0]) * vertices.size();
		const void* vertexBufferData = vertices.data();
		meshGpuData->pVertexBuffer = CBufferManager::CreateVertexBuffer(vertexBufferData, vertexBufferSize);

		const uint32 indexBufferSize = sizeof(indices[0]) * indices.size();
		const void* indexBufferData = indices.data();
		meshGpuData->pIndexBuffer = CBufferManager::CreateIndexBuffer(indexBufferData, indexBufferSize);
		meshGpuData->indexCount = static_cast<uint32>(pMesh->GetIndices().size());

		m_meshCache[pMesh->GetId()] = std::move(meshGpuData);

		LOG_INFO("MeshGpuData created and cached for Mesh ID: " << pMesh->GetId());

		return m_meshCache[pMesh->GetId()].get();
	}
} // BetterThanNothing