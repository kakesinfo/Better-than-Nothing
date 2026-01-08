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

#pragma once

#include "Module_Data/Graphics/DirectionalLightData.hpp"
#include "Module_Data/Graphics/PointLightData.hpp"
#include <Module_Math/Matrix4.hpp>
#include <ResourceManager/ResourceId.hpp>

namespace BetterThanNothing
{
	struct MeshGpuData;
	struct TextureGpuData;
	struct MeshData;
	struct ModelResource;

	class Matrix4;
	class RenderCache;
	class ResourceManager;

	struct RenderListItem
	{
		ResourceId meshId;
		ResourceId textureId;
		Matrix4			transform;
	};

	class RenderList
	{
	public:
		explicit RenderList(RenderCache* renderCache, ResourceManager* resourceManager) : m_renderCache(renderCache), m_resourceManager(resourceManager) {}
		~RenderList() = default;

		void SubmitModel(const std::string& modelPath, const Matrix4& transform, bool preTransform = false);
		void SubmitModel(const ModelResource* modelResource, const Matrix4& transform);
		void SubmitModel(ResourceId modelId, const Matrix4& transform);
		void SubmitMesh(const MeshData& mesh, const Matrix4& transform);
		void SubmitMesh(ResourceId meshId, ResourceId textureId, const Matrix4& transform);
		void Submit(const RenderListItem& item);

		void SetDirectionalLight(const DirectionalLightData& data);
		void AddPointLight(const PointLightData& data);

		[[nodiscard]]
		RenderListItem Get(std::uint32_t index) const;

		const DirectionalLightData&		   GetDirectionalLight() const;
		std::uint32_t					   GetPointLightCount() const;
		const std::vector<PointLightData>& GetPointLights() const;
		const PointLightData&			   GetPointLight(std::uint32_t index) const;

		void Clear();

		[[nodiscard]] std::uint32_t Size() const;
		[[nodiscard]] bool			IsEmpty() const;

	private:
		RenderCache*				m_renderCache;
		ResourceManager*			m_resourceManager;
		std::vector<RenderListItem> m_list;

		std::optional<DirectionalLightData> m_directionalLight;
		std::vector<PointLightData>			m_pointLights;
	};
}
