/*
 *	Better than Nothing
 *	Copyright (C) 2026 kakesinfo
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

#include <Common/ResourceId.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

namespace BetterThanNothing
{
	struct MeshData;

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

	struct MeshResourceCache;
	struct TextureResourceCache;

	struct ShaderGpuData;
	class TextureLoader;
	class ShaderLoader;
	enum class EShaderType;
	class RenderingDevice;
	class ResourceImporter;

	class ResourceManager
	{
	public:
		explicit ResourceManager(RenderingDevice* renderingDevice);
		~ResourceManager();

		static std::unique_ptr<ResourceManager> Create(RenderingDevice* renderingDevice);

	private:
		ResourceId GenerateUniqueResourceId();

	public:
		ResourceId LoadTexture(const std::string& path);
		ResourceId LoadModel(const std::string& path, bool preTransform = true);
		ResourceId LoadShader(const std::string& path, EShaderType type);

		MeshResourceCache*	  GetMeshData(ResourceId id);
		TextureResourceCache* GetTextureData(ResourceId id);
		ModelResource*		  GetModel(ResourceId id);
		ShaderGpuData*		  GetShader(ResourceId id);

	private:
		RenderingDevice*													  m_renderingDevice;
		std::unique_ptr<ShaderLoader>										  m_shaderLoader;
		std::unique_ptr<ResourceImporter> 									  m_importer;
		std::unordered_map<ResourceId, std::unique_ptr<MeshResourceCache>>	  m_meshes;
		std::unordered_map<ResourceId, std::unique_ptr<TextureResourceCache>> m_textures;
		std::unordered_map<ResourceId, std::unique_ptr<ShaderGpuData>>		  m_shaders;
		std::unordered_map<ResourceId, ModelResource>						  m_models;
		std::unordered_map<std::string, ResourceId>							  m_texturesLUT;
		std::unordered_map<std::string, ResourceId>							  m_modelsLUT;
		std::unordered_map<std::string, ResourceId>							  m_shadersLUT;
		std::vector<ResourceId>												  m_freeIds;
	};
}
