#pragma once

#include <deque>

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	struct RenderQueueItem
	{
		CMesh* pMesh;
		glm::mat4 worldTransform;
	};

	class CRenderQueue
	{
	public:
		CRenderQueue();
		~CRenderQueue();

		void Render(CMesh* pMesh, const glm::mat4& worldTransform);
		void Render(const RenderQueueItem& item);

		RenderQueueItem Pop();

		void Clear();

		[[nodiscard]] size_t Size() const;
		[[nodiscard]] bool IsEmpty() const;

	private:
		std::pmr::deque<RenderQueueItem> m_queue;
	};
}
