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

#pragma once

namespace BetterThanNothing
{
	struct MeshGpuData;
	struct TextureGpuData;
	struct ShaderGpuData;
	struct MaterialGpuData;
	struct MeshData;
	struct TextureData;
	struct ShaderData;
	struct VulkanContext;

	class TextureLoader;
	class ShaderLoader;

	class RenderCache
	{
	public:
		explicit RenderCache(VulkanContext* context);
		~RenderCache();

		MeshGpuData*	GetOrCreateMeshGpuData(const MeshData* mesh);
		TextureGpuData* GetOrCreateTextureGpuData(const TextureData& textureData);
		ShaderGpuData*	GetOrCreateShaderGpuData(const ShaderData& shaderData);

	private:
		std::unordered_map<std::uint64_t, std::unique_ptr<MeshGpuData>>		m_meshCache;
		std::unordered_map<std::uint64_t, std::unique_ptr<MaterialGpuData>> m_materialCache;
		std::unordered_map<std::string, std::unique_ptr<TextureGpuData>>	m_textureCache;
		std::unordered_map<std::string, std::unique_ptr<ShaderGpuData>>		m_shaderCache;

		VulkanContext*				   m_context;
		std::unique_ptr<TextureLoader> m_textureLoader;
		std::unique_ptr<ShaderLoader>  m_shaderLoader;
	};
} // BetterThanNothing