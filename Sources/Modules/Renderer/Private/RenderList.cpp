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

#include "Renderer/RenderList.hpp"
#include "ResourceManager/ResourceManager.hpp"

#include <Module_Data/Graphics/MeshData.hpp>

namespace BetterThanNothing
{
	void RenderList::SubmitModel(const std::string& modelPath, const Matrix4& transform, const bool preTransform)
	{
		SubmitModel(m_resourceManager->LoadModel(modelPath, preTransform), transform);
	}

	void RenderList::SubmitModel(const ResourceId modelId, const Matrix4& transform)
	{
		SubmitModel(m_resourceManager->GetModel(modelId), transform);
	}

	void RenderList::SubmitModel(const ModelResource* modelResource, const Matrix4& transform)
	{
		for (const auto& [meshId, textureId] : modelResource->meshes)
			SubmitMesh(meshId, textureId, transform);
	}

	void RenderList::SubmitMesh(const MeshData& mesh, const Matrix4& transform)
	{
		const ResourceId meshId = m_resourceManager->LoadMesh(mesh);
		const ResourceId textureId = m_resourceManager->LoadTexture(mesh.texturePath);
		SubmitMesh(meshId, textureId, transform);
	}

	void RenderList::SubmitMesh(ResourceId meshId, ResourceId textureId, const Matrix4& transform)
	{
		Submit({.meshId = meshId, .textureId = textureId, .transform = transform});
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
