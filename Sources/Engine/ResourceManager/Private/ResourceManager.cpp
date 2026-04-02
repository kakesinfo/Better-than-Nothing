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

	ResourceId ResourceManager::LoadModel(const std::string& path, bool preTransform)
	{
		if (const auto it = m_modelsLUT.find(path); it != m_modelsLUT.end())
			return it->second;

		const ImportedModel importedModel = m_importer->ImportModel(path, preTransform);

		ModelResource model;
		model.id = GenerateUniqueResourceId();

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

			if (mesh.materialId.has_value())
			{
				const ImportedMaterial material = importedModel.materials.at(mesh.materialId.value());

				if (material.diffuseTexturePath.has_value())
					meshResource.textureId = LoadTexture(material.diffuseTexturePath.value());
			}

			model.meshes.push_back(meshResource);
		}

		m_modelsLUT[path] = model.id;
		m_models[model.id] = model;

		Logger::Log<INFO>("MeshResourceCache created and cached for {} submeshes", model.meshes.size());

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
		if (!m_meshes.contains(id))
			throw std::runtime_error("No MeshResourceCache for ResourceId: " + std::to_string(id));
		return m_meshes[id].get();
	}

	TextureResourceCache* ResourceManager::GetTextureData(const ResourceId id)
	{
		if (!m_textures.contains(id))
			throw std::runtime_error("No TextureResourceCache for ResourceId: " + std::to_string(id));
		return m_textures[id].get();
	}

	ModelResource* ResourceManager::GetModelData(const ResourceId id)
	{
		if (!m_models.contains(id))
			throw std::runtime_error("No ModelResource for ResourceId: " + std::to_string(id));
		return &m_models[id];
	}

	ShaderResourceCache* ResourceManager::GetShaderData(ResourceId id)
	{
		if (!m_shaders.contains(id))
			throw std::runtime_error("No ShaderResourceCache for ResourceId: " + std::to_string(id));
		return m_shaders[id].get();
	}

	PipelineResourceCache* ResourceManager::GetPipelineData(const std::string& name)
	{
		if (!m_pipelinesLUT.contains(name))
			throw std::runtime_error("No PipelineResourceCache for ResourceId: " + name);
		return m_pipelines[m_pipelinesLUT[name]].get();
	}
}
