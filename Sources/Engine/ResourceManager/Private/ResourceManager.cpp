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
#include <Common/Logger.hpp>
#include <Data/Graphics/MeshData.hpp>
#include <Data/Graphics/VertexData.hpp>
#include <RenderingApi/Core/ShaderModule.hpp>
#include <RenderingApi/Data/ShaderGpuData_Legacy.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <RenderingDevice/SamplerDescription.hpp>
#include <RenderingDevice/TextureDescription.hpp>
#include <ResourceImporter/ResourceImporter.hpp>
#include <ResourceManager/Loader/ShaderLoader.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>
#include <filesystem>

namespace BetterThanNothing
{
	ResourceManager::ResourceManager(RenderingDevice* renderingDevice) : m_renderingDevice(renderingDevice)
	{
		m_shaderLoader = std::make_unique<ShaderLoader>(m_renderingDevice);
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
		gpuData->descriptorSetHandle = m_renderingDevice->CreateTextureDescriptorSet(gpuData->textureHandle, gpuData->samplerHandle);

		const auto textureId = GenerateUniqueResourceId();

		m_textures[textureId] = std::move(gpuData);
		m_texturesLUT[texturePath] = textureId;

		Logger::Log<INFO>("TextureResourceCache created and cached for Texture: {}", texturePath);

		return textureId;
	}

	ResourceId ResourceManager::LoadShader_Legacy(const std::string& path, EShaderType type)
	{
		if (const auto it = m_shadersLUT.find(path); it != m_shadersLUT.end())
			return it->second;

		const auto shaderId = GenerateUniqueResourceId();

		m_shadersLegacy[shaderId] = m_shaderLoader->Load({type, path});
		m_shadersLUT[path] = shaderId;

		Logger::Log<DEBUG>("ShaderGpuData created and cached for Shader: {}", path);

		return shaderId;
	}

	MeshResourceCache* ResourceManager::GetMeshData(ResourceId id)
	{
		if (!m_meshes.contains(id))
			return nullptr;

		return m_meshes[id].get();
	}

	TextureResourceCache* ResourceManager::GetTextureData(const ResourceId id)
	{
		if (!m_textures.contains(id))
			return nullptr;

		return m_textures[id].get();
	}

	ModelResource* ResourceManager::GetModel(const ResourceId id)
	{
		if (!m_models.contains(id))
			return nullptr;

		return &m_models[id];
	}

	ShaderGpuData_Legacy* ResourceManager::GetShader_Legacy(const ResourceId id)
	{
		if (!m_shadersLegacy.contains(id))
			return nullptr;

		return m_shadersLegacy[id].get();
	}
}
