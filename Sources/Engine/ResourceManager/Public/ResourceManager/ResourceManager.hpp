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
#include <memory>
#include <unordered_map>
#include <vector>

namespace BetterThanNothing
{
	struct PipelineResourceCache;
	struct ShaderResourceCache;
	struct MaterialResourceCache;
	struct MaterialResource;
	struct MeshData;

	struct MeshResource
	{
		ResourceId id{0};
		ResourceId materialId{0};
		ResourceId textureId{0};
	};

	struct ModelNodeResource
	{
		std::string name;
		Matrix4 transform{Matrix4::Identity};
		std::vector<uint32_t> meshIndices;
		std::vector<uint32_t> children;
	};

	struct ModelResource
	{
		ResourceId				       id{0};
		std::vector<MeshResource>      meshes;
		std::vector<ResourceId>        materials;
		std::vector<ModelNodeResource> nodes;
		std::vector<uint32_t>          rootNodes;
	};

	struct MeshResourceCache;
	struct TextureResourceCache;

	struct ShaderGpuData_Legacy;
	class TextureLoader;
	class ShaderLoader;
	enum class EShaderType : uint32_t;
	class RenderingDevice;
	class ResourceImporter;
	class PipelineBlueprint;

	class ResourceManager
	{
	public:
		explicit ResourceManager(RenderingDevice* renderingDevice);
		~ResourceManager();

		static std::unique_ptr<ResourceManager> Create(RenderingDevice* renderingDevice);

	private:
		ResourceId GenerateUniqueResourceId();

	public:
		ResourceId LoadModel(const std::string& path, bool preTransform = true);
		ResourceId LoadTexture(const std::string& path);
		ResourceId LoadTextureFromMemory(const std::string& name, const uint8_t* data, uint32_t width, uint32_t height, uint32_t channels);
		ResourceId CreateMaterial(const MaterialResource& material);
		ResourceId GetOrCreateDefaultMaterial();
		ResourceId LoadShader(const std::string& path);
		ResourceId CreatePipeline(const std::string& name, const std::function<void(PipelineBlueprint*)>& callback);

		MeshResourceCache*	   GetMeshData(ResourceId id);
		TextureResourceCache*  GetTextureData(ResourceId id);
		ModelResource*		   GetModelData(ResourceId id);
		MaterialResourceCache* GetMaterialData(ResourceId id);
		ShaderResourceCache*   GetShaderData(ResourceId id);
		PipelineResourceCache* GetPipelineData(const std::string& name);

		ResourceId GetDefaultWhiteTexture();
		ResourceId GetDefaultNormalTexture();
		ResourceId GetDefaultMetallicRoughnessTexture();
		ResourceId GetDefaultBlackTexture();

	private:
		void EnsureDefaultTextures();

		RenderingDevice*													   m_renderingDevice;
		std::unique_ptr<ResourceImporter>									   m_importer;
		std::unordered_map<ResourceId, std::unique_ptr<MeshResourceCache>>	   m_meshes;
		std::unordered_map<ResourceId, std::unique_ptr<TextureResourceCache>>  m_textures;
		std::unordered_map<ResourceId, std::unique_ptr<MaterialResourceCache>> m_materials;
		std::unordered_map<ResourceId, std::unique_ptr<ShaderResourceCache>>   m_shaders;
		std::unordered_map<ResourceId, std::unique_ptr<PipelineResourceCache>> m_pipelines;
		std::unordered_map<ResourceId, ModelResource>						   m_models;
		std::unordered_map<std::string, ResourceId>							   m_texturesLUT;
		std::unordered_map<std::string, ResourceId>							   m_modelsLUT;
		std::unordered_map<std::string, ResourceId>							   m_shadersLUT;
		std::unordered_map<std::string, ResourceId>							   m_pipelinesLUT;
		std::vector<ResourceId>												   m_freeIds;

		ResourceId m_defaultWhiteTexture{0};
		ResourceId m_defaultNormalTexture{0};
		ResourceId m_defaultMetallicRoughnessTexture{0};
		ResourceId m_defaultBlackTexture{0};
		ResourceId m_defaultMaterial{0};
	};
}
