#pragma once

#include <deque>

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	struct RenderListItem
	{
		MeshGpuData*	pMeshGpuData;
		TextureGpuData*	pTextureGpuData;
		glm::mat4		transform;
	};

	class CRenderList
	{
	public:
		explicit CRenderList(CRenderCache* pRenderCache): m_pRenderCache(pRenderCache) {}
		~CRenderList() = default;

		void Submit(const ModelData* pModel, const std::string& texturePath, const glm::mat4& transform);
		void Submit(const MeshData* pMesh, const std::string& texturePath, const glm::mat4& transform);
		void Submit(const RenderListItem& item);

		[[nodiscard]]
		RenderListItem Get(size_t index) const;

		void Clear();

		[[nodiscard]] size_t Size() const;
		[[nodiscard]] bool IsEmpty() const;

	private:
		CRenderCache* 				m_pRenderCache;
		std::vector<RenderListItem>	m_list;
	};
}
