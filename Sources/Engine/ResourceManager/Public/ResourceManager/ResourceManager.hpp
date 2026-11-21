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
#include <Math/Matrix4.hpp>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace BetterThanNothing
{
	struct PipelineResourceCache;
	struct ShaderResourceCache;
	struct MeshResourceCache;
	struct TextureResourceCache;
	struct MaterialResourceCache;
	struct MeshData;
	enum class EAlphaMode : uint8_t;

	struct MeshResource
	{
		ResourceId id = InvalidResourceId;
		ResourceId textureId = InvalidResourceId;
		ResourceId materialId = InvalidResourceId;
	};

	struct MaterialCreateInfo
	{
		std::string name;
		ResourceId	diffuseTextureId = InvalidResourceId;
		ResourceId	normalTextureId = InvalidResourceId;
		EAlphaMode	alphaMode{};
		float		alphaCutoff = 0.5f;
		std::string pipelineName;
	};

	struct ModelNodeResource
	{
		std::string			  name;
		Matrix4				  localTransform = Matrix4::Identity;
		std::vector<uint32_t> meshIndices;
		std::vector<uint32_t> children;
	};

	struct ModelResource
	{
		ResourceId					   id = InvalidResourceId;
		std::vector<MeshResource>	   meshes;
		std::vector<ModelNodeResource> nodes;
		std::vector<uint32_t>		   rootNodes;
	};

	enum class EShaderType : uint32_t;
	class RenderingDevice;
	class ResourceImporter;
	class PipelineBlueprint;

	class ResourceManager
	{
	public:
		explicit ResourceManager(RenderingDevice* renderingDevice);
		~ResourceManager();

		ResourceManager(const ResourceManager&) = delete;
		ResourceManager& operator=(const ResourceManager&) = delete;
		ResourceManager(ResourceManager&&) = delete;
		ResourceManager& operator=(ResourceManager&&) = delete;

		static std::unique_ptr<ResourceManager> Create(RenderingDevice* renderingDevice);

	private:
		ResourceId GenerateUniqueResourceId();

	public:
		ResourceId LoadModel(const std::string& path, bool preTransform = true);
		ResourceId LoadTexture(const std::string& path);
		ResourceId GetDefaultTexture();
		ResourceId GetDefaultNormalTexture();
		ResourceId CreateMaterial(const MaterialCreateInfo& info);
		ResourceId LoadShader(const std::string& path);
		ResourceId CreateMesh(const MeshData& meshData);
		ResourceId CreatePipeline(const std::string& name, const std::function<void(PipelineBlueprint*)>& callback);

		void Clear();

		[[nodiscard]] MeshResourceCache*	 GetMeshData(ResourceId id) const;
		[[nodiscard]] TextureResourceCache*	 GetTextureData(ResourceId id) const;
		[[nodiscard]] MaterialResourceCache* GetMaterialData(ResourceId id) const;
		[[nodiscard]] const ModelResource*	 GetModelData(ResourceId id) const;
		[[nodiscard]] ShaderResourceCache*	 GetShaderData(ResourceId id) const;
		[[nodiscard]] PipelineResourceCache* GetPipelineData(const std::string& name) const;

	private:
		ResourceId CreateSolidColorTexture(const std::string& cacheKey, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

		RenderingDevice*														m_renderingDevice;
		std::unique_ptr<ResourceImporter>										m_importer;
		std::unordered_map<ResourceId, std::unique_ptr<MeshResourceCache>>		m_meshes;
		std::unordered_map<ResourceId, std::unique_ptr<TextureResourceCache>>	m_textures;
		std::unordered_map<ResourceId, std::unique_ptr<MaterialResourceCache>>	m_materials;
		std::unordered_map<ResourceId, std::unique_ptr<ShaderResourceCache>>	m_shaders;
		std::unordered_map<ResourceId, std::unique_ptr<PipelineResourceCache>>	m_pipelines;
		std::unordered_map<ResourceId, ModelResource>							m_models;
		std::unordered_map<std::string, ResourceId>								m_texturesLUT;
		std::unordered_map<std::string, ResourceId>								m_materialsLUT;
		std::unordered_map<std::string, ResourceId>								m_modelsLUT;
		std::unordered_map<std::string, ResourceId>								m_shadersLUT;
		std::unordered_map<std::string, ResourceId>								m_pipelinesLUT;
		std::vector<ResourceId>													m_freeIds;
		ResourceId																m_nextResourceId = 0;
	};
}
