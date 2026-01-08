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

#include "Module_LegacyRenderer/RenderCache.hpp"
#include <ResourceManager/Loader/ShaderLoader.hpp>

#include <Module_Base/Logger.hpp>

#include <module_vulkan.hpp>

namespace BetterThanNothing
{
	RenderCache::RenderCache(VulkanContext* context) : m_context(context)
	{
		m_shaderLoader = std::make_unique<ShaderLoader>(context);
	}

	RenderCache::~RenderCache() = default;

	ShaderGpuData* RenderCache::GetOrCreateShaderGpuData(EVulkanShaderType type, const std::string& filePath)
	{
		const auto it = m_shaderCache.find(filePath);
		if (it != m_shaderCache.end())
			return it->second.get();

		m_shaderCache[filePath] = m_shaderLoader->Load({type, filePath});

		Logger::Log<DEBUG>("ShaderGpuData created and cached for Shader: {}", filePath);

		return m_shaderCache[filePath].get();
	}
}
