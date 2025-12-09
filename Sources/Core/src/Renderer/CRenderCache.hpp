#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class CRenderCache
	{
	public:
		CRenderCache(VulkanContext* pContext);
		~CRenderCache() = default;

		MeshGpuData* GetOrCreateMeshGpuData(const MeshData* pMesh);
		TextureGpuData* GetOrCreateTextureGpuData(const TextureData* pTexture);

	private:
		std::unordered_map<uint64, std::unique_ptr<MeshGpuData>> m_meshCache;
		std::unordered_map<uint64, std::unique_ptr<MaterialGpuData>> m_materialCache;
		std::unordered_map<std::string, std::unique_ptr<TextureGpuData>> m_textureCache;

		VulkanContext* m_pContext;
		std::unique_ptr<CTextureLoader> m_textureLoader;
	};
} // BetterThanNothing