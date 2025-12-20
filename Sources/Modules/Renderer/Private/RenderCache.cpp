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

#include "Module_Renderer/RenderCache.hpp"
#include "Module_Renderer/Data/ShaderData.hpp"
#include "Module_Renderer/Resources/ShaderLoader.hpp"
#include "Module_Renderer/Resources/TextureLoader.hpp"

#include <Module_Base/Logger.hpp>
#include <Module_Data/Graphics/MeshData.hpp>
#include <Module_Data/Graphics/TextureData.hpp>

#include <module_vulkan.hpp>

namespace BetterThanNothing
{
	RenderCache::RenderCache(VulkanContext* context) : m_context(context)
	{
		m_textureLoader = std::make_unique<TextureLoader>(context);
		m_shaderLoader = std::make_unique<ShaderLoader>(context);
	}

	RenderCache::~RenderCache() = default;

	MeshGpuData* RenderCache::GetOrCreateMeshGpuData(const MeshData* mesh)
	{
		const auto it = m_meshCache.find(mesh->id);
		if (it != m_meshCache.end())
			return it->second.get();

		auto meshGpuData = std::make_unique<MeshGpuData>();

		if (mesh->vertices.empty() || mesh->indices.empty())
			throw std::runtime_error("Mesh has no vertices or indices!");

		const std::uint32_t vertexBufferSize = sizeof(mesh->vertices[0]) * mesh->vertices.size();
		const void*			vertexBufferData = mesh->vertices.data();
		meshGpuData->vertexBuffer = BufferManager::CreateVertexBuffer(vertexBufferData, vertexBufferSize);

		const std::uint32_t indexBufferSize = sizeof(mesh->indices[0]) * mesh->indices.size();
		const void*			indexBufferData = mesh->indices.data();
		meshGpuData->indexBuffer = BufferManager::CreateIndexBuffer(indexBufferData, indexBufferSize);
		meshGpuData->indexCount = static_cast<std::uint32_t>(mesh->indices.size());

		m_meshCache[mesh->id] = std::move(meshGpuData);

		Logger::Log<DEBUG>("MeshGpuData created and cached for Mesh ID: {}", mesh->id);

		return m_meshCache[mesh->id].get();
	}

	TextureGpuData* RenderCache::GetOrCreateTextureGpuData(const TextureData& textureData)
	{
		const auto it = m_textureCache.find(textureData.filePath);
		if (it != m_textureCache.end())
			return it->second.get();

		m_textureCache[textureData.filePath] = m_textureLoader->Load(textureData.filePath);

		Logger::Log<DEBUG>("TextureGpuData created and cached for Texture: {}", textureData.filePath);

		return m_textureCache[textureData.filePath].get();
	}

	ShaderGpuData* RenderCache::GetOrCreateShaderGpuData(const ShaderData& shaderData)
	{
		const auto it = m_shaderCache.find(shaderData.filePath);
		if (it != m_shaderCache.end())
			return it->second.get();

		m_shaderCache[shaderData.filePath] = m_shaderLoader->Load(shaderData);

		Logger::Log<DEBUG>("ShaderGpuData created and cached for Shader: {}", shaderData.filePath);

		return m_shaderCache[shaderData.filePath].get();
	}
}
