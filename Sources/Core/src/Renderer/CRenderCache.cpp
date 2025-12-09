#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CRenderCache::CRenderCache(VulkanContext *pContext): m_pContext(pContext)
	{
		m_textureLoader = std::make_unique<CTextureLoader>(pContext);
	}

	MeshGpuData* CRenderCache::GetOrCreateMeshGpuData(const MeshData* pMesh)
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

	TextureGpuData* CRenderCache::GetOrCreateTextureGpuData(const TextureData* pTexture)
	{
		const auto it = m_textureCache.find(pTexture->filePath);
		if (it != m_textureCache.end())
			return it->second.get();

		m_textureCache[pTexture->filePath] = m_textureLoader->Load(pTexture->filePath);

		LOG_INFO("TextureGpuData created and cached for Texture: " << pTexture->filePath);

		return m_textureCache[pTexture->filePath].get();
	}
}