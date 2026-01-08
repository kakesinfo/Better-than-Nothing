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

#include "ResourceId.hpp"
#include <Module_Data/Graphics/MeshData.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

namespace BetterThanNothing
{
	struct MeshResource
	{
		ResourceId id;
		ResourceId textureId;
	};

	struct ModelResource
	{
		ResourceId				  id;
		std::vector<MeshResource> meshes;
	};

	struct VulkanContext;
	struct MeshGpuData;
	struct TextureGpuData;
	class TextureLoader;

	class ResourceManager
	{
	public:
		explicit ResourceManager(VulkanContext* context);
		~ResourceManager();

	private:
		ResourceId GenerateUniqueResourceId();

	public:
		ResourceId LoadMesh(const MeshData& mesh);
		ResourceId LoadTexture(const std::string& path);
		ResourceId LoadModel(const std::string& path, bool preTransform = true);

		MeshGpuData*	GetMeshGpuData(ResourceId id);
		TextureGpuData* GetTextureGpuData(ResourceId id);
		ModelResource*	GetModel(ResourceId id);

	private:
		VulkanContext*													m_context;
		std::unique_ptr<TextureLoader>									m_textureLoader;
		std::unordered_map<ResourceId, std::unique_ptr<MeshGpuData>>	m_meshes;
		std::unordered_map<ResourceId, std::unique_ptr<TextureGpuData>> m_textures;
		std::unordered_map<ResourceId, ModelResource>					m_models;
		std::unordered_map<std::string, ResourceId>						m_texturesLUT;
		std::unordered_map<std::string, ResourceId>						m_modelsLUT;
		std::vector<ResourceId>											m_freeIds;
	};
}
