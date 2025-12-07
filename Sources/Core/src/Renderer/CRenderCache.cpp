#include "CRenderCache.hpp"

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	MeshGpuData* CRenderCache::GetOrCreateMeshGpuData(MeshData* pMesh)
	{
		const auto it = m_meshCache.find(pMesh->id);
		if (it != m_meshCache.end())
			return it->second.get();

		auto meshGpuData = std::make_unique<MeshGpuData>();

		if (pMesh->vertices.empty() || pMesh->indices.empty())
			throw std::runtime_error("Mesh has no vertices or indices!");

		const uint32 vertexBufferSize = sizeof(pMesh->vertices[0]) * pMesh->vertices.size();
		const void* vertexBufferData = pMesh->vertices.data();
		meshGpuData->pVertexBuffer = CBufferManager::CreateVertexBuffer(vertexBufferData, vertexBufferSize);

		const uint32 indexBufferSize = sizeof(pMesh->indices[0]) * pMesh->indices.size();
		const void* indexBufferData = pMesh->indices.data();
		meshGpuData->pIndexBuffer = CBufferManager::CreateIndexBuffer(indexBufferData, indexBufferSize);
		meshGpuData->indexCount = static_cast<uint32>(pMesh->indices.size());

		m_meshCache[pMesh->id] = std::move(meshGpuData);

		LOG_INFO("MeshGpuData created and cached for Mesh ID: " << pMesh->id);

		return m_meshCache[pMesh->id].get();
	}
} // BetterThanNothing