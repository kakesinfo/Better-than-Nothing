#include "CRenderQueue.hpp"

namespace BetterThanNothing
{
	CRenderQueue::CRenderQueue() = default;
	CRenderQueue::~CRenderQueue() = default;

	void CRenderQueue::Render(CMesh *pMesh, const glm::mat4 &worldTransform)
	{
		Render({pMesh, worldTransform});
	}

	void CRenderQueue::Render(const RenderQueueItem &item)
	{
		m_queue.push_back(item);
	}

	RenderQueueItem CRenderQueue::Pop()
	{
		if (m_queue.empty())
			throw std::runtime_error("Render queue is empty!");

		const RenderQueueItem item = m_queue.front();
		m_queue.pop_front();

		return item;
	}

	void CRenderQueue::Clear()
	{
		m_queue.clear();
	}

	size_t CRenderQueue::Size() const
	{
		return m_queue.size();
	}

	bool CRenderQueue::IsEmpty() const
	{
		return m_queue.empty();
	}
}