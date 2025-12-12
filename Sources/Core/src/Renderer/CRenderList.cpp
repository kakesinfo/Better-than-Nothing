#include "CRenderList.hpp"

namespace BetterThanNothing
{

	void CRenderList::Submit(const ModelData* pModel, const std::string& texturePath, const glm::mat4& transform)
	{
		for (const MeshData& mesh : pModel->meshes)
		{
			Submit(&mesh, texturePath, transform);
		}
	}

	void CRenderList::Submit(const MeshData* pMesh, const std::string& texturePath, const glm::mat4& transform)
	{
		RenderListItem item;
		item.pMeshGpuData = m_pRenderCache->GetOrCreateMeshGpuData(pMesh);
		item.pTextureGpuData = m_pRenderCache->GetOrCreateTextureGpuData({texturePath});
		item.transform = transform;

		Submit(item);
	}

	void CRenderList::Submit(const RenderListItem &item)
	{
		m_list.push_back(item);
	}

	RenderListItem CRenderList::Get(const size_t index) const
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