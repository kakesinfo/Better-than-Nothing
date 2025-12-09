#include "CRenderList.hpp"

namespace BetterThanNothing
{
	CRenderList::CRenderList() = default;
	CRenderList::~CRenderList() = default;

	void CRenderList::Render(MeshData *pMesh, const glm::mat4& modelTransform, const std::string& texturePath)
	{
		Render({pMesh, modelTransform, texturePath});
	}

	void CRenderList::Render(const RenderQueueItem &item)
	{
		m_list.push_back(item);
	}

	RenderQueueItem CRenderList::Get(const size_t index) const
	{
		return m_list[index];
	}

	void CRenderList::Clear()
	{
		m_list.clear();
	}

	size_t CRenderList::Size() const
	{
		return m_list.size();
	}

	bool CRenderList::IsEmpty() const
	{
		return m_list.empty();
	}
}