#include "Module_Renderer/CRenderList.hpp"
#include "Module_Renderer/CRenderCache.hpp"

#include <Module_Data/Graphics/MeshData.hpp>
#include <Module_Data/Graphics/ModelData.hpp>
#include <Module_Data/Graphics/TextureData.hpp>

namespace BetterThanNothing
{

	void CRenderList::Submit(const ModelData* pModel, const glm::mat4& transform, const std::string& texturePath)
	{
		for (const MeshData& mesh : pModel->meshes)
		{
			MeshData texturedMesh = mesh;
			texturedMesh.texturePath = texturePath;
			Submit(&texturedMesh, transform);
		}
	}

	void CRenderList::Submit(const ModelData* pModel, const glm::mat4& transform)
	{
		for (const MeshData& mesh : pModel->meshes)
		{
			Submit(&mesh, transform);
		}
	}

	void CRenderList::Submit(const MeshData* pMesh, const glm::mat4& transform)
	{
		RenderListItem item;

		if (!pMesh->texturePath.empty()) {
			item.pTextureGpuData = m_pRenderCache->GetOrCreateTextureGpuData({pMesh->texturePath});
		}

		item.pMeshGpuData = m_pRenderCache->GetOrCreateMeshGpuData(pMesh);
		item.transform = transform;

		Submit(item);
	}

	void CRenderList::Submit(const RenderListItem &item)
	{
		m_list.push_back(item);
	}

	RenderListItem CRenderList::Get(const std::uint32_t index) const
	{
		return m_list[index];
	}

	void CRenderList::Clear()
	{
		m_list.clear();
	}

	std::uint32_t CRenderList::Size() const
	{
		return m_list.size();
	}

	bool CRenderList::IsEmpty() const
	{
		return m_list.empty();
	}
}