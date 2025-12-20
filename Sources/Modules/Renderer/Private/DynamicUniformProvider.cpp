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

#include "Module_Renderer/DynamicUniformProvider.hpp"
#include "Module_Renderer/Data/DynamicUniformData.hpp"

#include <Module_Base/Logger.hpp>
#include <Module_Vulkan/VulkanContext.hpp>
#include <Module_Vulkan/VulkanDevice.hpp>
#include <Module_Vulkan/Manager/BufferManager.hpp>

namespace BetterThanNothing
{
	DynamicUniformProvider::DynamicUniformProvider(VulkanContext* context, const std::uint32_t maxObjects) : m_context(context),
		m_maxObjects(maxObjects)
	{
		m_uniformAlignment = CalculateUniformAlignment();

		const std::uint32_t count = m_context->vkSettings.maxFramesInFlight;
		const std::uint32_t bufferSize = m_maxObjects * m_uniformAlignment;

		for (std::uint32_t i = 0; i < count; i++) {
			m_context->vkFrames[i].dynamicUniformBuffer = BufferManager::CreateUniformBuffer(bufferSize);
		}

		Logger::Log<SUCCESS>("Vulkan dynamic uniforms: ok (count: {})", count);
	}

	std::uint32_t DynamicUniformProvider::CalculateUniformAlignment() const
	{
		const std::uint32_t minUboAlignment = m_context->vkDevice->GetMinUniformBufferOffsetAlignment();

		std::uint32_t dynamicAlignment = sizeof(DynamicUniformData);
		if (minUboAlignment > 0)
			dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);

		return dynamicAlignment;
	}
}
