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

#include <Common/AssetPath.hpp>
#include <Common/Logger.hpp>
#include <Data/Graphics/MeshData.hpp>
#include <Data/Graphics/VertexData.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <RenderingDevice/SamplerDescription.hpp>
#include <RenderingDevice/TextureDescription.hpp>
#include <ResourceImporter/ResourceImporter.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>
#include <array>
#include <filesystem>
#include <limits>

namespace BetterThanNothing
{
	static EAlphaMode ConvertAlphaMode(const EImportedAlphaMode mode)
	{
		switch (mode)
		{
			case EImportedAlphaMode::Mask:
				return EAlphaMode::Mask;
			case EImportedAlphaMode::Blend:
				return EAlphaMode::Blend;
			case EImportedAlphaMode::Opaque:
			case EImportedAlphaMode::NONE:
			default:
				return EAlphaMode::Opaque;
		}
	}

	ResourceManager::ResourceManager(RenderingDevice* renderingDevice) : m_renderingDevice(renderingDevice)
	{
		m_importer = std::make_unique<ResourceImporter>();
	}

	ResourceManager::~ResourceManager()
	{
		Clear();
	}

	std::unique_ptr<ResourceManager> ResourceManager::Create(RenderingDevice* renderingDevice)
	{
		return std::make_unique<ResourceManager>(renderingDevice);
	}

	ResourceId ResourceManager::GenerateUniqueResourceId()
	{
		if (!m_freeIds.empty())
		{
			const auto resourceId = m_freeIds.back();
			m_freeIds.pop_back();
			return resourceId;
		}

		return ++m_nextResourceId;
	}

	void ResourceManager::Clear()
	{
		if (!m_renderingDevice)
			return;

		m_renderingDevice->WaitIdle();

		for (const auto& [id, pipeline] : m_pipelines)
		{
			if (pipeline->pipeline.IsValid())
				m_renderingDevice->DestroyPipeline(pipeline->pipeline);
		}
		m_pipelines.clear();
		m_pipelinesLUT.clear();

		for (const auto& [id, shader] : m_shaders)
		{
			if (shader->module.IsValid())
				m_renderingDevice->DestroyShader(shader->module);
		}
		m_shaders.clear();
		m_shadersLUT.clear();

		for (const auto& [id, mesh] : m_meshes)
		{
			if (mesh->vertexBuffer.IsValid())
				m_renderingDevice->DestroyBuffer(mesh->vertexBuffer);
			if (mesh->indexBuffer.IsValid())
				m_renderingDevice->DestroyBuffer(mesh->indexBuffer);
		}
		m_meshes.clear();

		for (const auto& [id, material] : m_materials)
		{
			if (material->descriptorSetHandle.IsValid())
				m_renderingDevice->DestroyDescriptorSet(material->descriptorSetHandle);
		}
		m_materials.clear();
		m_materialsLUT.clear();

		for (const auto& [id, texture] : m_textures)
		{
			if (texture->descriptorSetHandle.IsValid())
				m_renderingDevice->DestroyDescriptorSet(texture->descriptorSetHandle);
			if (texture->textureHandle.IsValid())
				m_renderingDevice->DestroyTexture(texture->textureHandle);
		}
		m_textures.clear();
		m_texturesLUT.clear();

		m_models.clear();
		m_modelsLUT.clear();
		m_freeIds.clear();
	}

	ResourceId ResourceManager::CreateSolidColorTexture(const std::string& cacheKey, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
	{
		if (const auto it = m_texturesLUT.find(cacheKey); it != m_texturesLUT.end())
			return it->second;

		const std::array<uint8_t, 4> pixel = {r, g, b, a};

		auto gpuData = std::make_unique<TextureResourceCache>();
		gpuData->width = 1;
		gpuData->height = 1;
		gpuData->channels = 4;
		gpuData->size = 4;

		TextureFromDataDescription desc{};
		desc.data = pixel.data();
		desc.dataSize = pixel.size();
		desc.width = 1;
		desc.height = 1;
		desc.channels = 4;
		desc.generateMipmaps = false;

		gpuData->textureHandle = m_renderingDevice->CreateTextureFromData(desc);
		gpuData->samplerHandle = m_renderingDevice->GetOrCreateTextureSampler(SamplerDescription{});

		const ResourceId textureId = GenerateUniqueResourceId();
		m_textures[textureId] = std::move(gpuData);
		m_texturesLUT[cacheKey] = textureId;
		return textureId;
	}

	ResourceId ResourceManager::GetDefaultTexture()
	{
		const std::string resolvedDefault = AssetPath::Resolve(".assets/default_texture.png");
		std::error_code	  ec;
		if (std::filesystem::exists(resolvedDefault, ec))
			return LoadTexture(resolvedDefault);

		return CreateSolidColorTexture("__builtin_white__", 255, 255, 255, 255);
	}

	ResourceId ResourceManager::GetDefaultNormalTexture()
	{
		return CreateSolidColorTexture("__builtin_flat_normal__", 128, 128, 255, 255);
	}

	ResourceId ResourceManager::CreateMaterial(const MaterialCreateInfo& info)
	{
		const ResourceId diffuseId = info.diffuseTextureId != InvalidResourceId ? info.diffuseTextureId : GetDefaultTexture();
		const ResourceId normalId = info.normalTextureId != InvalidResourceId ? info.normalTextureId : GetDefaultNormalTexture();
		const std::string pipelineName = !info.pipelineName.empty()
											 ? info.pipelineName
											 : (info.alphaMode == EAlphaMode::Blend ? "Forward::Transparent" : "Forward::Opaque");

		const std::string cacheKey = std::to_string(diffuseId) + "|" + std::to_string(normalId) + "|" + std::to_string(static_cast<uint8_t>(info.alphaMode)) + "|" + pipelineName;
		if (const auto it = m_materialsLUT.find(cacheKey); it != m_materialsLUT.end())
			return it->second;

		const TextureResourceCache* diffuseTex = GetTextureData(diffuseId);
		const TextureResourceCache* normalTex = GetTextureData(normalId);

		const std::array<TextureBindingUpdate, 2> bindings = {{
			{.binding = 0, .textureHandle = diffuseTex->textureHandle, .samplerHandle = diffuseTex->samplerHandle},
			{.binding = 1, .textureHandle = normalTex->textureHandle, .samplerHandle = normalTex->samplerHandle},
		}};

		auto material = std::make_unique<MaterialResourceCache>();
		material->name = info.name.empty() ? ("Material_" + std::to_string(m_materials.size())) : info.name;
		material->alphaMode = info.alphaMode;
		material->alphaCutoff = info.alphaCutoff;
		material->diffuseTextureId = diffuseId;
		material->normalTextureId = normalId;
		material->pipelineName = pipelineName;
		material->descriptorSetHandle = m_renderingDevice->CreateCombinedImageDescriptorSet(bindings, "Material");

		const ResourceId materialId = GenerateUniqueResourceId();
		m_materials[materialId] = std::move(material);
		m_materialsLUT[cacheKey] = materialId;

		return materialId;
	}

	ResourceId ResourceManager::CreateMesh(const MeshData& meshData)
	{
		if (meshData.vertices.empty() || meshData.indices.empty())
			throw std::runtime_error("Cannot create GPU mesh from empty MeshData");

		const ResourceId meshId = GenerateUniqueResourceId();
		auto			 meshGpuData = std::make_unique<MeshResourceCache>();

		const size_t vertexBufferSize = sizeof(meshData.vertices[0]) * meshData.vertices.size();
		meshGpuData->vertexBuffer = m_renderingDevice->CreateVertexBuffer(meshData.vertices.data(), vertexBufferSize);
		meshGpuData->vertexCount = static_cast<uint32_t>(meshData.vertices.size());

		const size_t indexBufferSize = sizeof(meshData.indices[0]) * meshData.indices.size();
		meshGpuData->indexBuffer = m_renderingDevice->CreateIndexBuffer(meshData.indices.data(), indexBufferSize);
		meshGpuData->indexCount = static_cast<uint32_t>(meshData.indices.size());

		Vector3 minBounds(std::numeric_limits<float>::max());
		Vector3 maxBounds(std::numeric_limits<float>::lowest());
		for (const auto& v : meshData.vertices)
		{
			minBounds = Vector3::Min(minBounds, v.position);
			maxBounds = Vector3::Max(maxBounds, v.position);
		}
		meshGpuData->boundsMin = minBounds;
		meshGpuData->boundsMax = maxBounds;

		m_meshes[meshId] = std::move(meshGpuData);
		return meshId;
	}

	ResourceId ResourceManager::LoadModel(const std::string& path, bool preTransform)
	{
		const std::string resolvedPath = AssetPath::Resolve(path);
		const std::string cacheKey = resolvedPath + (preTransform ? "|pre" : "|raw");
		if (const auto it = m_modelsLUT.find(cacheKey); it != m_modelsLUT.end())
			return it->second;

		const ImportedModel importedModel = m_importer->ImportModel(resolvedPath, preTransform);

		// Batch all vertex/index/texture GPU uploads for this model into minimal command buffer submissions
		RenderingDevice::UploadBatchScope uploadBatch(m_renderingDevice);

		ModelResource model;
		model.id = GenerateUniqueResourceId();
		model.meshes.reserve(importedModel.meshes.size());

		const ResourceId defaultTexId = GetDefaultTexture();
		const ResourceId defaultNormalId = GetDefaultNormalTexture();

		for (const auto& mesh : importedModel.meshes)
		{
			MeshResource meshResource{};

			if (!mesh.vertices.empty() && !mesh.indices.empty())
			{
				const ResourceId meshId = GenerateUniqueResourceId();
				auto			 meshGpuData = std::make_unique<MeshResourceCache>();

				const size_t vertexBufferSize = sizeof(mesh.vertices[0]) * mesh.vertices.size();
				meshGpuData->vertexBuffer = m_renderingDevice->CreateVertexBuffer(mesh.vertices.data(), vertexBufferSize);
				meshGpuData->vertexCount = static_cast<uint32_t>(mesh.vertices.size());

				const size_t indexBufferSize = sizeof(mesh.indices[0]) * mesh.indices.size();
				meshGpuData->indexBuffer = m_renderingDevice->CreateIndexBuffer(mesh.indices.data(), indexBufferSize);
				meshGpuData->indexCount = static_cast<uint32_t>(mesh.indices.size());
				meshGpuData->boundsMin = mesh.min;
				meshGpuData->boundsMax = mesh.max;

				m_meshes[meshId] = std::move(meshGpuData);
				meshResource.id = meshId;
			}

			MaterialCreateInfo matInfo{};
			matInfo.diffuseTextureId = defaultTexId;
			matInfo.normalTextureId = defaultNormalId;
			matInfo.alphaMode = EAlphaMode::Opaque;

			if (mesh.materialId.has_value() && mesh.materialId.value() < importedModel.materials.size())
			{
				const ImportedMaterial& importedMat = importedModel.materials[mesh.materialId.value()];
				matInfo.alphaMode = ConvertAlphaMode(importedMat.alphaMode);
				matInfo.alphaCutoff = importedMat.alphaCutoff;

				if (importedMat.diffuseTexturePath.has_value())
					matInfo.diffuseTextureId = LoadTexture(importedMat.diffuseTexturePath.value());

				if (importedMat.normalTexturePath.has_value())
				{
					std::error_code ec;
					const std::string resolvedNorm = AssetPath::Resolve(importedMat.normalTexturePath.value());
					if (std::filesystem::exists(resolvedNorm, ec))
						matInfo.normalTextureId = LoadTexture(resolvedNorm);
				}
			}

			meshResource.textureId = matInfo.diffuseTextureId;
			meshResource.materialId = CreateMaterial(matInfo);
			model.meshes.push_back(meshResource);
		}

		model.nodes.reserve(importedModel.nodes.size());
		for (const auto& importedNode : importedModel.nodes)
		{
			ModelNodeResource node{};
			node.name = importedNode.name;
			node.localTransform = importedNode.transform;
			node.meshIndices = importedNode.meshes;
			node.children = importedNode.children;
			model.nodes.push_back(std::move(node));
		}
		model.rootNodes = importedModel.rootNodes;

		m_modelsLUT[cacheKey] = model.id;
		const ResourceId modelId = model.id;
		m_models[modelId] = std::move(model);

		Logger::Log<INFO>("ModelResource cached: {} ({} submeshes, {} nodes)", resolvedPath, m_models[modelId].meshes.size(), m_models[modelId].nodes.size());

		return modelId;
	}

	ResourceId ResourceManager::LoadTexture(const std::string& path)
	{
		if (path.empty())
			return GetDefaultTexture();

		const std::string texturePath = AssetPath::Resolve(path);

		if (const auto it = m_texturesLUT.find(texturePath); it != m_texturesLUT.end())
			return it->second;

		ImportedTexture importedTexture = m_importer->ImportTexture(texturePath);

		auto gpuData = std::make_unique<TextureResourceCache>();
		gpuData->channels = static_cast<int32_t>(importedTexture.channels);
		gpuData->size = static_cast<int32_t>(importedTexture.size);
		gpuData->width = static_cast<int32_t>(importedTexture.width);
		gpuData->height = static_cast<int32_t>(importedTexture.height);

		TextureFromDataDescription desc{};
		desc.data = importedTexture.data.data();
		desc.dataSize = importedTexture.size;
		desc.width = importedTexture.width;
		desc.height = importedTexture.height;
		desc.channels = importedTexture.channels;

		gpuData->textureHandle = m_renderingDevice->CreateTextureFromData(desc);
		gpuData->samplerHandle = m_renderingDevice->GetOrCreateTextureSampler(SamplerDescription{});

		const auto textureId = GenerateUniqueResourceId();
		m_textures[textureId] = std::move(gpuData);
		m_texturesLUT[texturePath] = textureId;

		Logger::Log<INFO>("TextureResourceCache created and cached for Texture: {}", texturePath);

		return textureId;
	}

	ResourceId ResourceManager::LoadShader(const std::string& path)
	{
		const std::string resolvedPath = AssetPath::Resolve(path);
		if (const auto it = m_shadersLUT.find(resolvedPath); it != m_shadersLUT.end())
			return it->second;

		ImportedShader importedShader = m_importer->ImportShader(resolvedPath);

		auto gpuData = std::make_unique<ShaderResourceCache>();
		gpuData->name = std::filesystem::path(resolvedPath).filename().string();
		gpuData->type = importedShader.type;
		gpuData->module = m_renderingDevice->CreateShader({gpuData->name, importedShader.type, std::move(importedShader.spirv)});

		const auto shaderId = GenerateUniqueResourceId();
		m_shaders[shaderId] = std::move(gpuData);
		m_shadersLUT[resolvedPath] = shaderId;

		Logger::Log<INFO>("ShaderResourceCache created and cached for Shader: {}", resolvedPath);

		return shaderId;
	}

	ResourceId ResourceManager::CreatePipeline(const std::string& name, const std::function<void(PipelineBlueprint*)>& callback)
	{
		if (const auto it = m_pipelinesLUT.find(name); it != m_pipelinesLUT.end())
			return it->second;

		PipelineBlueprint blueprint;
		callback(&blueprint);

		PipelineDescription desc{};
		desc.vertexLayout = blueprint.GetVertexLayout();
		desc.layoutName = blueprint.GetPipelineLayout();
		desc.depthTest = blueprint.GetDepthTest();
		desc.depthWrite = blueprint.GetDepthWrite();
		desc.blendEnable = blueprint.GetBlendEnable();
		desc.alphaToCoverage = blueprint.GetAlphaToCoverage();
		desc.wireframe = blueprint.GetWireframe();
		desc.cullMode = blueprint.GetCullMode();
		desc.shaders.reserve(blueprint.GetShaders().size());
		for (const auto& shaderId : blueprint.GetShaders())
			desc.shaders.push_back(GetShaderData(shaderId)->module);

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

	MeshResourceCache* ResourceManager::GetMeshData(const ResourceId id) const
	{
		const auto it = m_meshes.find(id);
		if (it == m_meshes.end())
			throw std::runtime_error("No MeshResourceCache for ResourceId: " + std::to_string(id));
		return it->second.get();
	}

	TextureResourceCache* ResourceManager::GetTextureData(const ResourceId id) const
	{
		const auto it = m_textures.find(id);
		if (it == m_textures.end())
			throw std::runtime_error("No TextureResourceCache for ResourceId: " + std::to_string(id));
		return it->second.get();
	}

	MaterialResourceCache* ResourceManager::GetMaterialData(const ResourceId id) const
	{
		const auto it = m_materials.find(id);
		if (it == m_materials.end())
			throw std::runtime_error("No MaterialResourceCache for ResourceId: " + std::to_string(id));
		return it->second.get();
	}

	const ModelResource* ResourceManager::GetModelData(const ResourceId id) const
	{
		const auto it = m_models.find(id);
		if (it == m_models.end())
			throw std::runtime_error("No ModelResource for ResourceId: " + std::to_string(id));
		return &it->second;
	}

	ShaderResourceCache* ResourceManager::GetShaderData(const ResourceId id) const
	{
		const auto it = m_shaders.find(id);
		if (it == m_shaders.end())
			throw std::runtime_error("No ShaderResourceCache for ResourceId: " + std::to_string(id));
		return it->second.get();
	}

	PipelineResourceCache* ResourceManager::GetPipelineData(const std::string& name) const
	{
		const auto lutIt = m_pipelinesLUT.find(name);
		if (lutIt == m_pipelinesLUT.end())
			throw std::runtime_error("No PipelineResourceCache for name: " + name);
		return m_pipelines.at(lutIt->second).get();
	}
}
