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

#include "ResourceManager/ResourceManager.hpp"
#include "ResourceManager/Loader/ShaderLoader.hpp"
#include "ResourceManager/Loader/TextureLoader.hpp"
#include "ResourceManager/MeshGpuData.hpp"
#include "ResourceManager/TextureGpuData.hpp"

#define TINYOBJLOADER_IMPLEMENTATION

#include <future>
#include <tiny_obj_loader.h>

#include <Module_Base/Logger.hpp>
#include <Module_Data/Graphics/VertexData.hpp>
#include <RenderingApi/Data/ShaderGpuData.hpp>
#include <RenderingApi/Deprecated/VulkanContext.hpp>
#include <RenderingApi/Deprecated/VulkanShaderModule.hpp>

namespace BetterThanNothing
{
	ResourceManager::ResourceManager(VulkanContext* context) : m_context(context)
	{
		m_textureLoader = std::make_unique<TextureLoader>(context);
		m_shaderLoader = std::make_unique<ShaderLoader>(context);
	}

	ResourceManager::~ResourceManager() = default;

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

		tinyobj::attrib_t				 attrib;
		std::vector<tinyobj::shape_t>	 shapes;
		std::vector<tinyobj::material_t> materials;
		std::string						 warn;
		std::string						 err;

		std::filesystem::path parentFolder = std::filesystem::path(path).parent_path();

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), parentFolder.string().c_str()))
			throw std::runtime_error(err);

		if (!warn.empty())
			Logger::Log<WARNING>("TinyObjLoader warning: {}", warn);

		float minX = std::numeric_limits<float>::max();
		float minY = std::numeric_limits<float>::max();
		float minZ = std::numeric_limits<float>::max();

		float maxX = std::numeric_limits<float>::lowest();
		float maxY = std::numeric_limits<float>::lowest();
		float maxZ = std::numeric_limits<float>::lowest();

		std::vector<MeshData> loadedMeshes;
		for (const auto& shape : shapes)
		{
			MeshData														  currentMesh;
			std::unordered_map<VertexData, uint32_t, VertexHash, VertexEqual> uniqueVertices{};

			if (!shape.mesh.material_ids.empty())
			{
				const int materialId = shape.mesh.material_ids[0];

				if (materialId >= 0 && materialId < materials.size())
				{
					const auto& material = materials[materialId];

					if (!material.diffuse_texname.empty())
					{
						std::filesystem::path texPath = parentFolder / material.diffuse_texname;
						currentMesh.texturePath = texPath.string();
					}
				}
			}

			for (const auto& index : shape.mesh.indices)
			{
				VertexData vertex{};

				float rawX = attrib.vertices[3 * index.vertex_index + 0];
				float rawY = attrib.vertices[3 * index.vertex_index + 1];
				float rawZ = attrib.vertices[3 * index.vertex_index + 2];

				// Coordinate system transformation:
				// If preTransform is false, convert from Y-up (OBJ/Blender) to Z-up (engine) coordinate system.
				// This maps (X, Y, Z) -> (X, Z, -Y):
				//   - X remains the same.
				//   - Y becomes Z (up axis).
				//   - Z becomes -Y (to maintain right-handedness).
				if (preTransform)
				{
					vertex.position = {rawX, rawY, rawZ};
				}
				else
				{
					vertex.position = {rawX, rawZ, -rawY};
				}

				vertex.color = {1.0f, 1.0f, 1.0f};

				if (!attrib.texcoords.empty() && index.texcoord_index != -1)
				{
					vertex.uv = {attrib.texcoords[2 * index.texcoord_index + 0], 1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
				}
				else
				{
					vertex.uv = {0.0f, 0.0f};
				}

				// Load normals
				if (!attrib.normals.empty() && index.normal_index != -1)
				{
					auto xNormal = attrib.normals[3 * index.normal_index + 0];
					auto yNormal = attrib.normals[3 * index.normal_index + 1];
					auto zNormal = attrib.normals[3 * index.normal_index + 2];

					vertex.normal = preTransform ? Vector3{xNormal, yNormal, zNormal} : Vector3{xNormal, zNormal, -yNormal};
				}
				else
				{
					vertex.normal = {0.0f, 0.0f, 0.0f};
				}

				if (!uniqueVertices.contains(vertex))
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(currentMesh.vertices.size());

					if (vertex.position.GetX() < minX)
						minX = vertex.position.GetX();
					if (vertex.position.GetY() < minY)
						minY = vertex.position.GetY();
					if (vertex.position.GetZ() < minZ)
						minZ = vertex.position.GetZ();

					if (vertex.position.GetX() > maxX)
						maxX = vertex.position.GetX();
					if (vertex.position.GetY() > maxY)
						maxY = vertex.position.GetY();
					if (vertex.position.GetZ() > maxZ)
						maxZ = vertex.position.GetZ();

					currentMesh.vertices.push_back(vertex);
				}

				currentMesh.indices.push_back(uniqueVertices[vertex]);
			}

			loadedMeshes.push_back(currentMesh);
		}

		float centerX = (minX + maxX) / 2.0f;
		float centerY = (minY + maxY) / 2.0f;
		float centerZ = (minZ + maxZ) / 2.0f;

		ModelResource model;
		for (auto& mesh : loadedMeshes)
		{
			for (auto& vert : mesh.vertices)
				vert.position -= {centerX, centerY, centerZ};

			MeshResource meshResource{};
			meshResource.id = LoadMesh(mesh);
			meshResource.textureId = LoadTexture(mesh.texturePath);

			model.meshes.push_back(meshResource);
		}

		model.id = GenerateUniqueResourceId();
		m_modelsLUT[path] = model.id;
		m_models[model.id] = model;

		return model.id;
	}

	ResourceId ResourceManager::LoadMesh(const MeshData& mesh)
	{
		auto meshId = GenerateUniqueResourceId();
		auto meshGpuData = std::make_unique<MeshGpuData>();

		if (mesh.vertices.empty() || mesh.indices.empty())
			throw std::runtime_error("Mesh has no vertices or indices!");

		const std::uint32_t vertexBufferSize = sizeof(mesh.vertices[0]) * mesh.vertices.size();
		const void*			vertexBufferData = mesh.vertices.data();
		meshGpuData->vertexBuffer = std::make_unique<VulkanVertexBuffer>(m_context, vertexBufferSize, vertexBufferData);
		meshGpuData->vertexCount = static_cast<std::uint32_t>(mesh.vertices.size());

		const std::uint32_t indexBufferSize = sizeof(mesh.indices[0]) * mesh.indices.size();
		const void*			indexBufferData = mesh.indices.data();
		meshGpuData->indexBuffer = std::make_unique<VulkanIndexBuffer>(m_context, indexBufferSize, indexBufferData);
		meshGpuData->indexCount = static_cast<std::uint32_t>(mesh.indices.size());

		m_meshes[meshId] = std::move(meshGpuData);

		Logger::Log<DEBUG>("MeshGpuData created and cached for Mesh ID: {}", meshId);

		return meshId;
	}

	ResourceId ResourceManager::LoadTexture(const std::string& path)
	{
		std::string texturePath = path;
		if (texturePath.empty())
			texturePath = "../../../Assets/Textures/default_texture.png";

		if (const auto it = m_texturesLUT.find(texturePath); it != m_texturesLUT.end())
			return it->second;

		const auto textureId = GenerateUniqueResourceId();

		m_textures[textureId] = m_textureLoader->Load(texturePath);
		m_texturesLUT[texturePath] = textureId;

		Logger::Log<DEBUG>("TextureGpuData created and cached for Texture: {}", texturePath);

		return textureId;
	}

	ResourceId ResourceManager::LoadShader(const std::string& path, EShaderType type)
	{
		if (const auto it = m_shadersLUT.find(path); it != m_shadersLUT.end())
			return it->second;

		const auto shaderId = GenerateUniqueResourceId();

		m_shaders[shaderId] = m_shaderLoader->Load({type, path});
		m_shadersLUT[path] = shaderId;

		Logger::Log<DEBUG>("ShaderGpuData created and cached for Shader: {}", path);

		return shaderId;
	}

	MeshGpuData* ResourceManager::GetMeshGpuData(ResourceId id)
	{
		if (!m_meshes.contains(id))
			return nullptr;

		return m_meshes[id].get();
	}

	TextureGpuData* ResourceManager::GetTextureGpuData(const ResourceId id)
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

	ShaderGpuData* ResourceManager::GetShader(const ResourceId id)
	{
		if (!m_shaders.contains(id))
			return nullptr;

		return m_shaders[id].get();
	}
}
