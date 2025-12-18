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

#include "module_vulkan.hpp"

namespace BetterThanNothing
{
	VulkanContext* CBufferManager::m_context = nullptr;

	void CBufferManager::Initialize(VulkanContext* context)
	{
		m_context = context;
	}

	void CBufferManager::Shutdown()
	{
		m_context = nullptr;
	}

	std::unique_ptr<CVulkanVertexBuffer> CBufferManager::CreateVertexBuffer(const void* data, const std::uint32_t dataSize)
	{
		return std::make_unique<CVulkanVertexBuffer>(m_context, dataSize, data);
	}

	std::unique_ptr<CVulkanIndexBuffer> CBufferManager::CreateIndexBuffer(const void* data, const std::uint32_t dataSize)
	{
		return std::make_unique<CVulkanIndexBuffer>(m_context, dataSize, data);
	}

	std::unique_ptr<CVulkanUniformBuffer> CBufferManager::CreateUniformBuffer(const std::uint32_t dataSize)
	{
		return std::make_unique<CVulkanUniformBuffer>(m_context, dataSize);
	}
}