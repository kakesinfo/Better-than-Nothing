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

namespace BetterThanNothing
{
	VulkanContext* BufferManager::m_context = nullptr;

	void BufferManager::Initialize(VulkanContext* context)
	{
		m_context = context;
	}

	void BufferManager::Shutdown()
	{
		m_context = nullptr;
	}

	std::unique_ptr<VulkanVertexBuffer> BufferManager::CreateVertexBuffer(const void*		  data,
																		  const std::uint32_t dataSize)
	{
		return std::make_unique<VulkanVertexBuffer>(m_context, dataSize, data);
	}

	std::unique_ptr<VulkanIndexBuffer> BufferManager::CreateIndexBuffer(const void* data, const std::uint32_t dataSize)
	{
		return std::make_unique<VulkanIndexBuffer>(m_context, dataSize, data);
	}

	std::unique_ptr<VulkanStorageBuffer> BufferManager::CreateStorageBuffer(const std::uint32_t dataSize)
	{
		return std::make_unique<VulkanStorageBuffer>(m_context, dataSize);
	}

	std::unique_ptr<VulkanUniformBuffer> BufferManager::CreateUniformBuffer(const std::uint32_t dataSize)
	{
		return std::make_unique<VulkanUniformBuffer>(m_context, dataSize);
	}
}
