#include "Module_Renderer/CRenderCache.hpp"
#include "Module_Renderer/Resources/CTextureLoader.hpp"
#include "Module_Renderer/Resources/CShaderLoader.hpp"
#include "Module_Renderer/Data/ShaderData.hpp"

#include <module_vulkan.hpp>

#include <Module_Base/CLogger.hpp>
#include <Module_Data/Graphics/MeshData.hpp>
#include <Module_Data/Graphics/TextureData.hpp>

namespace BetterThanNothing
{
	CRenderCache::CRenderCache(VulkanContext *pContext): m_pContext(pContext)
	{
		m_pTextureLoader = std::make_unique<CTextureLoader>(pContext);
		m_pShaderLoader = std::make_unique<CShaderLoader>(pContext);
	}

	CRenderCache::~CRenderCache() = default;

	MeshGpuData* CRenderCache::GetOrCreateMeshGpuData(const MeshData* pMesh)
	{
		const auto it = m_meshCache.find(pMesh->id);
		if (it != m_meshCache.end())
			return it->second.get();

		auto meshGpuData = std::make_unique<MeshGpuData>();

		if (pMesh->vertices.empty() || pMesh->indices.empty())
			throw std::runtime_error("Mesh has no vertices or indices!");

		const std::uint32_t vertexBufferSize = sizeof(pMesh->vertices[0]) * pMesh->vertices.size();
		const void* vertexBufferData = pMesh->vertices.data();
		meshGpuData->pVertexBuffer = CBufferManager::CreateVertexBuffer(vertexBufferData, vertexBufferSize);

		const std::uint32_t indexBufferSize = sizeof(pMesh->indices[0]) * pMesh->indices.size();
		const void* indexBufferData = pMesh->indices.data();
		meshGpuData->pIndexBuffer = CBufferManager::CreateIndexBuffer(indexBufferData, indexBufferSize);
		meshGpuData->indexCount = static_cast<std::uint32_t>(pMesh->indices.size());

		m_meshCache[pMesh->id] = std::move(meshGpuData);

		CLogger::Log<Debug>("MeshGpuData created and cached for Mesh ID: {}", pMesh->id);

		return m_meshCache[pMesh->id].get();
	}

	TextureGpuData* CRenderCache::GetOrCreateTextureGpuData(const TextureData& textureData)
	{
		const auto it = m_textureCache.find(textureData.filePath);
		if (it != m_textureCache.end())
			return it->second.get();

		m_textureCache[textureData.filePath] = m_pTextureLoader->Load(textureData.filePath);

		CLogger::Log<Debug>("TextureGpuData created and cached for Texture: {}", textureData.filePath);

		return m_textureCache[textureData.filePath].get();
	}

	ShaderGpuData* CRenderCache::GetOrCreateShaderGpuData(const ShaderData& shaderData)
	{
		const auto it = m_shaderCache.find(shaderData.filePath);
		if (it != m_shaderCache.end())
			return it->second.get();

		m_shaderCache[shaderData.filePath] = m_pShaderLoader->Load(shaderData);

		CLogger::Log<Debug>("ShaderGpuData created and cached for Shader: {}", shaderData.filePath);

		return m_shaderCache[shaderData.filePath].get();
	}
}
