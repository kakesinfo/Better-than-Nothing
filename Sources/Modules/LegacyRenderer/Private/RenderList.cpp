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

#include "Module_LegacyRenderer/RenderList.hpp"
#include "Module_LegacyRenderer/RenderCache.hpp"

#include <Module_Data/Graphics/MeshData.hpp>
#include <Module_Data/Graphics/ModelData.hpp>
#include <Module_Data/Graphics/TextureData.hpp>

namespace BetterThanNothing
{

	void RenderList::Submit(const ModelData* model, const Matrix4& transform, const std::string& texturePath)
	{
		for (const MeshData& mesh : model->meshes)
		{
			MeshData texturedMesh = mesh;
			texturedMesh.texturePath = texturePath;
			Submit(&texturedMesh, transform);
		}
	}

	void RenderList::Submit(const ModelData* model, const Matrix4& transform)
	{
		for (const MeshData& mesh : model->meshes)
		{
			Submit(&mesh, transform);
		}
	}

	void RenderList::Submit(const MeshData* mesh, const Matrix4& transform)
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

	void RenderList::SetDirectionalLight(const DirectionalLightData& data)
	{
		m_directionalLight = data;
	}

	void RenderList::AddPointLight(const PointLightData& data)
	{
		m_pointLights.push_back(data);
	}

	RenderListItem RenderList::Get(const std::uint32_t index) const
	{
		return m_list[index];
	}

	const DirectionalLightData& RenderList::GetDirectionalLight() const
	{
		if (!m_directionalLight.has_value())
			throw std::runtime_error("No directional light");
		return m_directionalLight.value();
	}

	std::uint32_t RenderList::GetPointLightCount() const
	{
		return m_pointLights.size();
	}

	const std::vector<PointLightData>& RenderList::GetPointLights() const
	{
		return m_pointLights;
	}

	const PointLightData& RenderList::GetPointLight(const std::uint32_t index) const
	{
		return m_pointLights[index];
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