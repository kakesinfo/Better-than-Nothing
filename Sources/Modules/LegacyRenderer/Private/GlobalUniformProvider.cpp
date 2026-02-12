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

#include "Module_LegacyRenderer/GlobalUniformProvider.hpp"
#include "Module_LegacyRenderer/Data/GlobalUniformData.hpp"

#include <Module_Base/Logger.hpp>
#include <RenderingApi/Manager/BufferManager.hpp>
#include <RenderingApi/VulkanContext.hpp>

namespace BetterThanNothing
{
	GlobalUniformProvider::GlobalUniformProvider(VulkanContext* context) : m_context(context)
	{
		const std::uint32_t count = m_context->vkSettings.maxFramesInFlight;
		for (std::uint32_t i = 0; i < count; i++)
		{
			m_context->vkFrames[i].globalUniformBuffer = BufferManager::CreateUniformBuffer<GlobalUniformData>();
		}

		Logger::Log<SUCCESS>("Vulkan global uniforms: ok (count: {})", count);
	}
}
