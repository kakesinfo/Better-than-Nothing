/*
 *	Better than Nothing
 *	Copyright (C) 2025 kakesinfo
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 * 	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "Module_Renderer/RenderList.hpp"
#include "Module_Renderer/RenderCache.hpp"

#include <Module_Data/Graphics/MeshData.hpp>
#include <Module_Data/Graphics/ModelData.hpp>
#include <Module_Data/Graphics/TextureData.hpp>

namespace BetterThanNothing
{

	void RenderList::Submit(const ModelData* model, const glm::mat4& transform, const std::string& texturePath)
	{
		for (const MeshData& mesh : model->meshes)
		{
			MeshData texturedMesh = mesh;
			texturedMesh.texturePath = texturePath;
			Submit(&texturedMesh, transform);
		}
	}

	void RenderList::Submit(const ModelData* model, const glm::mat4& transform)
	{
		for (const MeshData& mesh : model->meshes)
		{
			Submit(&mesh, transform);
		}
	}

	void RenderList::Submit(const MeshData* mesh, const glm::mat4& transform)
	{
		RenderListItem item{};

		item.meshGpuData = m_renderCache->GetOrCreateMeshGpuData(mesh);
		item.textureGpuData = m_renderCache->GetOrCreateTextureGpuData({mesh->texturePath});
		item.transform = transform;

		Submit(item);
	}

	void RenderList::Submit(const RenderListItem& item)
	{
		m_list.push_back(item);
	}

	RenderListItem RenderList::Get(const std::uint32_t index) const
	{
		return m_list[index];
	}

	void RenderList::Clear()
	{
		m_list.clear();
	}

	std::uint32_t RenderList::Size() const
	{
		return m_list.size();
	}

	bool RenderList::IsEmpty() const
	{
		return m_list.empty();
	}
}