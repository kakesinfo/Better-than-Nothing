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

#include "ResourceManager/ResourceManager.hpp"

#include "RenderingDevice/PipelineDescription.hpp"
#include "RenderingDevice/ShaderDescription.hpp"
#include "ResourceManager/PipelineBlueprint.hpp"

#include <Common/Logger.hpp>
#include <Data/Graphics/VertexData.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <RenderingDevice/SamplerDescription.hpp>
#include <RenderingDevice/TextureDescription.hpp>
#include <ResourceImporter/ResourceImporter.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>
#include <filesystem>

namespace BetterThanNothing
{
	ResourceManager::ResourceManager(RenderingDevice* renderingDevice) : m_renderingDevice(renderingDevice)
	{
		m_importer = std::make_unique<ResourceImporter>();
	}

	ResourceManager::~ResourceManager() = default;

	std::unique_ptr<ResourceManager> ResourceManager::Create(RenderingDevice* renderingDevice)
	{
		return std::make_unique<ResourceManager>(renderingDevice);
	}

	ResourceId ResourceManager::GenerateUniqueResourceId()
	{
		static ResourceId lastResourceId = 0;

		if (!m_freeIds.empty())
		{
			const auto resourceId = m_freeIds.back();
			m_freeIds.pop_back();
			return resourceId;
		}

		return ++lastResourceId;
	}

	void ResourceManager::EnsureDefaultTextures()
	{
		if (m_defaultWhiteTexture != 0)
			return;

		const uint8_t whitePixel[4] = {255, 255, 255, 255};
		m_defaultWhiteTexture = LoadTextureFromMemory("__default_white__", whitePixel, 1, 1, 4);

		const uint8_t normalPixel[4] = {128, 128, 255, 255};
		m_defaultNormalTexture = LoadTextureFromMemory("__default_normal__", normalPixel, 1, 1, 4);

		const uint8_t metalRoughPixel[4] = {0, 255, 0, 255}; // Roughness = 1.0 (green), Metallic = 0.0 (blue)
		m_defaultMetallicRoughnessTexture = LoadTextureFromMemory("__default_metal_rough__", metalRoughPixel, 1, 1, 4);

		const uint8_t blackPixel[4] = {0, 0, 0, 255};
		m_defaultBlackTexture = LoadTextureFromMemory("__default_black__", blackPixel, 1, 1, 4);
	}

	ResourceId ResourceManager::GetDefaultWhiteTexture()
	{
		EnsureDefaultTextures();
		return m_defaultWhiteTexture;
	}

	ResourceId ResourceManager::GetDefaultNormalTexture()
	{
		EnsureDefaultTextures();
		return m_defaultNormalTexture;
	}

	ResourceId ResourceManager::GetDefaultMetallicRoughnessTexture()
	{
		EnsureDefaultTextures();
		return m_defaultMetallicRoughnessTexture;
	}

	ResourceId ResourceManager::GetDefaultBlackTexture()
	{
		EnsureDefaultTextures();
		return m_defaultBlackTexture;
	}

	ResourceId ResourceManager::LoadTextureFromMemory(const std::string& name, const uint8_t* data, uint32_t width, uint32_t height, uint32_t channels)
	{
		if (const auto it = m_texturesLUT.find(name); it != m_texturesLUT.end())
			return it->second;

		auto gpuData = std::make_unique<TextureResourceCache>();
		gpuData->channels = channels;
		gpuData->width = width;
		gpuData->height = height;
		gpuData->size = width * height * channels;

		TextureFromDataDescription desc{};
		desc.data = const_cast<uint8_t*>(data);
		desc.dataSize = gpuData->size;
		desc.width = width;
		desc.height = height;
		desc.channels = channels;

		gpuData->textureHandle = m_renderingDevice->CreateTextureFromData(desc);
		gpuData->samplerHandle = m_renderingDevice->GetOrCreateTextureSampler(SamplerDescription{});
		gpuData->descriptorSetHandle = m_renderingDevice->CreateTextureDescriptorSet(gpuData->textureHandle, gpuData->samplerHandle, "Textures");

		const auto textureId = GenerateUniqueResourceId();
		m_textures[textureId] = std::move(gpuData);
		m_texturesLUT[name] = textureId;

		return textureId;
	}

	ResourceId ResourceManager::CreateMaterial(const MaterialResource& material)
	{
		EnsureDefaultTextures();

		auto materialCache = std::make_unique<MaterialResourceCache>();
		materialCache->id = material.id != 0 ? material.id : GenerateUniqueResourceId();
		materialCache->material = material;
		materialCache->material.id = materialCache->id;

		ResourceId albedoId = material.albedoTextureId != 0 ? material.albedoTextureId : m_defaultWhiteTexture;
		ResourceId normalId = material.normalTextureId != 0 ? material.normalTextureId : m_defaultNormalTexture;
		ResourceId metalRoughId = material.metallicRoughnessTextureId != 0 ? material.metallicRoughnessTextureId : m_defaultMetallicRoughnessTexture;
		ResourceId occlusionId = material.occlusionTextureId != 0 ? material.occlusionTextureId : m_defaultWhiteTexture;
		ResourceId emissiveId = material.emissiveTextureId != 0 ? material.emissiveTextureId : m_defaultBlackTexture;

		materialCache->material.albedoTextureId = albedoId;
		materialCache->material.normalTextureId = normalId;
		materialCache->material.metallicRoughnessTextureId = metalRoughId;
		materialCache->material.occlusionTextureId = occlusionId;
		materialCache->material.emissiveTextureId = emissiveId;

		const auto albedoCache = GetTextureData(albedoId);
		const auto normalCache = GetTextureData(normalId);
		const auto metalRoughCache = GetTextureData(metalRoughId);
		const auto occlusionCache = GetTextureData(occlusionId);
		const auto emissiveCache = GetTextureData(emissiveId);

		std::array<Handle, 5> textures = {
			albedoCache->textureHandle,
			normalCache->textureHandle,
			metalRoughCache->textureHandle,
			occlusionCache->textureHandle,
			emissiveCache->textureHandle
		};

		// Create UBO for material factors
		materialCache->uniformBufferHandle = m_renderingDevice->CreateUniformBuffer(sizeof(MaterialUniformData));
		m_renderingDevice->CopyDataToBuffer(materialCache->uniformBufferHandle, &material.parameters, sizeof(MaterialUniformData));

		materialCache->descriptorSetHandle = m_renderingDevice->CreateMaterialDescriptorSet(
			textures,
			albedoCache->samplerHandle,
			materialCache->uniformBufferHandle,
			sizeof(MaterialUniformData),
			"Material"
		);

		const auto matId = materialCache->id;
		m_materials[matId] = std::move(materialCache);
		return matId;
	}

	ResourceId ResourceManager::GetOrCreateDefaultMaterial()
	{
		EnsureDefaultTextures();

		if (m_defaultMaterial != 0)
			return m_defaultMaterial;

		MaterialResource defaultMat{};
		defaultMat.name = "DefaultMaterial";
		defaultMat.albedoTextureId = m_defaultWhiteTexture;
		defaultMat.normalTextureId = m_defaultNormalTexture;
		defaultMat.metallicRoughnessTextureId = m_defaultMetallicRoughnessTexture;
		defaultMat.occlusionTextureId = m_defaultWhiteTexture;
		defaultMat.emissiveTextureId = m_defaultBlackTexture;
		defaultMat.parameters.baseColorFactor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		defaultMat.parameters.roughnessFactor = 1.0f;
		defaultMat.parameters.metallicFactor = 0.0f;
		defaultMat.parameters.emissiveFactor = Vector3(0.0f, 0.0f, 0.0f);

		m_defaultMaterial = CreateMaterial(defaultMat);
		return m_defaultMaterial;
	}

	MaterialResourceCache* ResourceManager::GetMaterialData(ResourceId id)
	{
		const auto it = m_materials.find(id);
		if (it != m_materials.end())
			return it->second.get();
		return GetMaterialData(GetOrCreateDefaultMaterial());
	}

	ResourceId ResourceManager::LoadModel(const std::string& path, bool preTransform)
	{
		EnsureDefaultTextures();

		const std::string cacheKey = path + (preTransform ? "_pre" : "_raw");
		if (const auto it = m_modelsLUT.find(cacheKey); it != m_modelsLUT.end())
			return it->second;

		const ImportedModel importedModel = m_importer->ImportModel(path, preTransform);

		ModelResource model;
		model.id = GenerateUniqueResourceId();

		// 1. Process and load materials
		std::vector<ResourceId> loadedMaterials;
		loadedMaterials.reserve(importedModel.materials.size());

		for (const auto& importedMat : importedModel.materials)
		{
			MaterialResource mat{};
			mat.name = importedMat.name;
			if (importedMat.diffuseTexturePath.has_value())
				mat.albedoTextureId = LoadTexture(importedMat.diffuseTexturePath.value());
			if (importedMat.normalTexturePath.has_value())
				mat.normalTextureId = LoadTexture(importedMat.normalTexturePath.value());
			if (importedMat.metallicRoughnessTexturePath.has_value())
				mat.metallicRoughnessTextureId = LoadTexture(importedMat.metallicRoughnessTexturePath.value());
			if (importedMat.occlusionTexturePath.has_value())
				mat.occlusionTextureId = LoadTexture(importedMat.occlusionTexturePath.value());
			if (importedMat.emissiveTexturePath.has_value())
				mat.emissiveTextureId = LoadTexture(importedMat.emissiveTexturePath.value());

			mat.parameters.baseColorFactor = importedMat.baseColorFactor;
			mat.parameters.metallicFactor = importedMat.metallicFactor;
			mat.parameters.roughnessFactor = importedMat.roughnessFactor;
			mat.parameters.emissiveFactor = importedMat.emissiveFactor;
			mat.parameters.alphaCutoff = importedMat.alphaCutoff;

			loadedMaterials.push_back(CreateMaterial(mat));
		}
		model.materials = loadedMaterials;

		// 2. Process meshes
		for (auto& mesh : importedModel.meshes)
		{
			MeshResource meshResource{};

			if (!mesh.vertices.empty() && !mesh.indices.empty())
			{
				auto meshId = GenerateUniqueResourceId();
				auto meshGpuData = std::make_unique<MeshResourceCache>();

				const uint32_t vertexBufferSize = sizeof(mesh.vertices[0]) * mesh.vertices.size();
				const void*	   vertexBufferData = mesh.vertices.data();
				meshGpuData->vertexBuffer = m_renderingDevice->CreateVertexBuffer(vertexBufferData, vertexBufferSize);
				meshGpuData->vertexCount = static_cast<uint32_t>(mesh.vertices.size());

				const uint32_t indexBufferSize = sizeof(mesh.indices[0]) * mesh.indices.size();
				const void*	   indexBufferData = mesh.indices.data();
				meshGpuData->indexBuffer = m_renderingDevice->CreateIndexBuffer(indexBufferData, indexBufferSize);
				meshGpuData->indexCount = static_cast<uint32_t>(mesh.indices.size());

				m_meshes[meshId] = std::move(meshGpuData);
				meshResource.id = meshId;
			}

			if (mesh.materialId.has_value() && mesh.materialId.value() < loadedMaterials.size())
			{
				meshResource.materialId = loadedMaterials[mesh.materialId.value()];
				const auto matCache = GetMaterialData(meshResource.materialId);
				meshResource.textureId = matCache->material.albedoTextureId;
			}
			else
			{
				meshResource.materialId = GetOrCreateDefaultMaterial();
				meshResource.textureId = m_defaultWhiteTexture;
			}

			model.meshes.push_back(meshResource);
		}

		// 3. Process nodes and hierarchy
		for (const auto& importedNode : importedModel.children)
		{
			ModelNodeResource nodeResource{};
			nodeResource.name = importedNode.name;
			nodeResource.transform = importedNode.transform;
			nodeResource.meshIndices = importedNode.meshes;
			nodeResource.children = importedNode.children;
			model.nodes.push_back(nodeResource);
		}
		model.rootNodes = importedModel.rootNodes;

		m_modelsLUT[cacheKey] = model.id;
		m_models[model.id] = model;

		Logger::Log<INFO>("MeshResourceCache created and cached for {} submeshes, {} materials", model.meshes.size(), model.materials.size());

		return model.id;
	}

	ResourceId ResourceManager::LoadTexture(const std::string& path)
	{
		std::string texturePath = path;
		if (texturePath.empty())
			texturePath = "../../../.assets/default_texture.png";

		if (const auto it = m_texturesLUT.find(texturePath); it != m_texturesLUT.end())
			return it->second;

		ImportedTexture importedTexture = m_importer->ImportTexture(texturePath);

		auto gpuData = std::make_unique<TextureResourceCache>();
		gpuData->channels = importedTexture.channels;
		gpuData->size = importedTexture.size;
		gpuData->width = importedTexture.width;
		gpuData->height = importedTexture.height;

		TextureFromDataDescription desc{};
		desc.data = importedTexture.data.data();
		desc.dataSize = gpuData->size;
		desc.width = gpuData->width;
		desc.height = gpuData->height;
		desc.channels = gpuData->channels;

		gpuData->textureHandle = m_renderingDevice->CreateTextureFromData(desc);
		gpuData->samplerHandle = m_renderingDevice->GetOrCreateTextureSampler(SamplerDescription{}); // Default sampler description
		gpuData->descriptorSetHandle = m_renderingDevice->CreateTextureDescriptorSet(gpuData->textureHandle, gpuData->samplerHandle, "Textures");

		const auto textureId = GenerateUniqueResourceId();
		m_textures[textureId] = std::move(gpuData);
		m_texturesLUT[texturePath] = textureId;

		Logger::Log<INFO>("TextureResourceCache created and cached for Texture: {}", texturePath);

		return textureId;
	}

	ResourceId ResourceManager::LoadShader(const std::string& path)
	{
		if (const auto it = m_shadersLUT.find(path); it != m_shadersLUT.end())
			return it->second;

		ImportedShader importedShader = m_importer->ImportShader(path);

		auto gpuData = std::make_unique<ShaderResourceCache>();
		gpuData->name = std::filesystem::path(path).filename().string();
		gpuData->type = importedShader.type;
		gpuData->module = m_renderingDevice->CreateShader({gpuData->name, importedShader.type, importedShader.data.data()});

		const auto shaderId = GenerateUniqueResourceId();
		m_shaders[shaderId] = std::move(gpuData);
		m_shadersLUT[path] = shaderId;

		Logger::Log<INFO>("ShaderResourceCache created and cached for Shader: {}", path);

		return shaderId;
	}

	ResourceId ResourceManager::CreatePipeline(const std::string& name, const std::function<void(PipelineBlueprint*)>& callback)
	{
		if (const auto it = m_pipelinesLUT.find(name); it != m_pipelinesLUT.end())
			return it->second;

		PipelineBlueprint blueprint;
		callback(&blueprint);

		PipelineDescription desc {};
		desc.vertexLayout = blueprint.GetVertexLayout();
		desc.layoutName = blueprint.GetPipelineLayout();
		desc.shaders.reserve(blueprint.GetShaders().size());
		for (const auto& shaderId : blueprint.GetShaders())
			desc.shaders.push_back(GetShaderData(shaderId)->module);

		// TODO: add more stuff to description using blueprint

		auto cache = std::make_unique<PipelineResourceCache>();
		cache->name = name;
		cache->layoutName = desc.layoutName;
		cache->pipeline = m_renderingDevice->CreatePipeline(desc);

		const auto pipelineId = GenerateUniqueResourceId();

		m_pipelines[pipelineId] = std::move(cache);
		m_pipelinesLUT[name] = pipelineId;

		Logger::Log<INFO>("PipelineResourceCache created and cached for Pipeline: {}", name);

		return pipelineId;
	}

	MeshResourceCache* ResourceManager::GetMeshData(ResourceId id)
	{
		const auto it = m_meshes.find(id);
		if (it == m_meshes.end())
			throw std::runtime_error("No MeshResourceCache for ResourceId: " + std::to_string(id));
		return it->second.get();
	}

	TextureResourceCache* ResourceManager::GetTextureData(const ResourceId id)
	{
		const auto it = m_textures.find(id);
		if (it == m_textures.end())
			throw std::runtime_error("No TextureResourceCache for ResourceId: " + std::to_string(id));
		return it->second.get();
	}

	ModelResource* ResourceManager::GetModelData(const ResourceId id)
	{
		const auto it = m_models.find(id);
		if (it == m_models.end())
			throw std::runtime_error("No ModelResource for ResourceId: " + std::to_string(id));
		return &it->second;
	}

	ShaderResourceCache* ResourceManager::GetShaderData(ResourceId id)
	{
		const auto it = m_shaders.find(id);
		if (it == m_shaders.end())
			throw std::runtime_error("No ShaderResourceCache for ResourceId: " + std::to_string(id));
		return it->second.get();
	}

	PipelineResourceCache* ResourceManager::GetPipelineData(const std::string& name)
	{
		const auto it = m_pipelinesLUT.find(name);
		if (it == m_pipelinesLUT.end())
			throw std::runtime_error("No PipelineResourceCache for ResourceId: " + name);
		return m_pipelines.at(it->second).get();
	}
}
