#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class CRenderCache
	{
	public:
		CRenderCache() = default;
		~CRenderCache() = default;

		MeshGpuData* GetOrCreateMeshGpuData(CMesh* pMesh);

	private:
		std::unordered_map<uint64, std::unique_ptr<MeshGpuData>> m_meshCache;
		std::unordered_map<uint64, std::unique_ptr<MaterialGpuData>> m_materialCache;
	};
} // BetterThanNothing