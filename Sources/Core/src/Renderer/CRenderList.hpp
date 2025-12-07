#pragma once

#include <deque>

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	struct RenderQueueItem
	{
		MeshData* pMesh;
		glm::mat4 modelTransform;
	};

	class CRenderList
	{
	public:
		CRenderList();
		~CRenderList();

		void Render(MeshData* pMesh, const glm::mat4& modelTransform);
		void Render(const RenderQueueItem& item);

		[[nodiscard]]
		RenderQueueItem Get(size_t index) const;

		void Clear();

		[[nodiscard]] size_t Size() const;
		[[nodiscard]] bool IsEmpty() const;

	private:
		std::vector<RenderQueueItem> m_list;
	};
}
