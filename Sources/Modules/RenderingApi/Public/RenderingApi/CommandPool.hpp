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
	class Device;
	class Queue;

	struct CommandPoolCreateInfo
	{
		Device* device;
		Queue*	queue;
	};

	class CommandPool
	{
	public:
		explicit CommandPool(const CommandPoolCreateInfo& createInfo);
		~CommandPool();

		CommandPool(const CommandPool&) = delete;
		CommandPool& operator=(const CommandPool&) = delete;
		CommandPool(CommandPool&& other) = delete;
		CommandPool& operator=(CommandPool&& other) = delete;

		[[nodiscard]] static std::unique_ptr<CommandPool> Create(const CommandPoolCreateInfo& createInfo);
		[[nodiscard]] VkCommandPool						  Handle() const;

	private:
		CommandPoolCreateInfo m_createInfo;
		VkCommandPool		  m_commandPool = VK_NULL_HANDLE;
	};
}
