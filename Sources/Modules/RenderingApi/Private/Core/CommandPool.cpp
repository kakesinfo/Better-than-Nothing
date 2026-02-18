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

#include "RenderingApi.hpp"

#include <Module_Base/Logger.hpp>

namespace BetterThanNothing
{
	CommandPool::CommandPool(const CommandPoolCreateInfo& createInfo) : m_createInfo(createInfo)
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = m_createInfo.queue->GetFamilyIndex();

		if (vkCreateCommandPool(m_createInfo.device->Handle(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
			throw std::runtime_error("failed to create command pool!");

		Logger::Log<SUCCESS>("Vulkan command pool (queueFamilyIndex: {}): ok", m_createInfo.queue->GetFamilyIndex());
	}

	CommandPool::~CommandPool()
	{
		vkDestroyCommandPool(m_createInfo.device->Handle(), m_commandPool, nullptr);
	}

	std::unique_ptr<CommandPool> CommandPool::Create(const CommandPoolCreateInfo& createInfo)
	{
		return std::make_unique<CommandPool>(createInfo);
	}

	VkCommandPool CommandPool::Handle() const
	{
		return m_commandPool;
	}
}
