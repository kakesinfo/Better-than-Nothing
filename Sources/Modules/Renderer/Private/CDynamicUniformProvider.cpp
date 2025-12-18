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

#include "Module_Renderer/CDynamicUniformProvider.hpp"
#include "Module_Renderer/Data/DynamicUniformData.hpp"

#include <Module_Base/CLogger.hpp>
#include <Module_Vulkan/VulkanContext.hpp>
#include <Module_Vulkan/VulkanFrame.hpp>
#include <Module_Vulkan/CVulkanDevice.hpp>
#include <Module_Vulkan/CVulkanUniformBuffer.hpp>
#include <Module_Vulkan/Manager/CBufferManager.hpp>

namespace BetterThanNothing
{
	CDynamicUniformProvider::CDynamicUniformProvider(VulkanContext* pContext, const std::uint32_t maxObjects) : m_pContext(pContext),
		m_maxObjects(maxObjects)
	{
		m_uniformAlignment = _calculateUniformAlignment();

		const std::uint32_t count = m_pContext->vkSettings.maxFramesInFlight;
		const std::uint32_t bufferSize = m_maxObjects * m_uniformAlignment;

		for (std::uint32_t i = 0; i < count; i++) {
			m_pContext->vkFrames[i].dynamicUniformBuffer = CBufferManager::CreateUniformBuffer(bufferSize);
		}

		CLogger::Log<Success>("Vulkan dynamic uniforms: ok (count: {})", count);
	}

	std::uint32_t CDynamicUniformProvider::_calculateUniformAlignment() const
	{
		const std::uint32_t minUboAlignment = m_pContext->vkDevice->GetMinUniformBufferOffsetAlignment();

		std::uint32_t dynamicAlignment = sizeof(DynamicUniformData);
		if (minUboAlignment > 0)
			dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);

		return dynamicAlignment;
	}
}
