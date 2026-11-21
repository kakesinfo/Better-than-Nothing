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
#include <Data/Graphics/MeshData.hpp>
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

		m_meshes[meshId] = std::move(meshGpuData);
		return meshId;
	}

	ResourceId ResourceManager::LoadModel(const std::string& path, bool preTransform)
	{
		const std::string cacheKey = path + (preTransform ? "|pre" : "|raw");
		if (const auto it = m_modelsLUT.find(cacheKey); it != m_modelsLUT.end())
			return it->second;

		const ImportedModel importedModel = m_importer->ImportModel(path, preTransform);

		ModelResource model;
		model.id = GenerateUniqueResourceId();
		model.meshes.reserve(importedModel.meshes.size());

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

				m_meshes[meshId] = std::move(meshGpuData);
				meshResource.id = meshId;
			}

			std::string diffusePath;
			if (mesh.materialId.has_value() && mesh.materialId.value() < importedModel.materials.size())
			{
				const ImportedMaterial& material = importedModel.materials[mesh.materialId.value()];
				if (material.diffuseTexturePath.has_value())
					diffusePath = material.diffuseTexturePath.value();
			}

			meshResource.textureId = LoadTexture(diffusePath);
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

		Logger::Log<INFO>("ModelResource cached: {} ({} submeshes, {} nodes)", path, m_models[modelId].meshes.size(), m_models[modelId].nodes.size());

		return modelId;
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
		gpuData->module = m_renderingDevice->CreateShader({gpuData->name, importedShader.type, std::move(importedShader.spirv)});

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

		PipelineDescription desc{};
		desc.vertexLayout = blueprint.GetVertexLayout();
		desc.layoutName = blueprint.GetPipelineLayout();
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
