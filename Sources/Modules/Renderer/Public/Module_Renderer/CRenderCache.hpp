#pragma once

namespace BetterThanNothing
{
	struct MeshGpuData;
	struct TextureGpuData;
	struct ShaderGpuData;
	struct MaterialGpuData;
	struct MeshData;
	struct TextureData;
	struct ShaderData;
	struct VulkanContext;

	class CTextureLoader;
	class CShaderLoader;

	class CRenderCache
	{
	public:
		explicit CRenderCache(VulkanContext* pContext);
		~CRenderCache() = default;

		MeshGpuData* GetOrCreateMeshGpuData(const MeshData* pMesh);
		TextureGpuData* GetOrCreateTextureGpuData(const TextureData& textureData);
		ShaderGpuData* GetOrCreateShaderGpuData(const ShaderData& shaderData);

	private:
		std::unordered_map<std::uint64_t, std::unique_ptr<MeshGpuData>>			m_meshCache;
		std::unordered_map<std::uint64_t, std::unique_ptr<MaterialGpuData>>		m_materialCache;
		std::unordered_map<std::string, std::unique_ptr<TextureGpuData>>	m_textureCache;
		std::unordered_map<std::string, std::unique_ptr<ShaderGpuData>>		m_shaderCache;

		VulkanContext* m_pContext;
		std::unique_ptr<CTextureLoader> m_pTextureLoader;
		std::unique_ptr<CShaderLoader> m_pShaderLoader;
	};
} // BetterThanNothing