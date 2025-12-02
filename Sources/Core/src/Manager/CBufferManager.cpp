//
// Created by acoezard on 11/25/24.
//

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanContext* CBufferManager::m_context = nullptr;

	void CBufferManager::Initialize(CVulkanContext* context)
	{
		m_context = context;
	}

	void CBufferManager::Shutdown()
	{
		m_context = nullptr;
	}

	std::unique_ptr<CVulkanVertexBuffer> CBufferManager::CreateVertexBuffer(const void* data, const uint32 dataSize)
	{
		return std::make_unique<CVulkanVertexBuffer>(m_context, dataSize, data);
	}

	std::unique_ptr<CVulkanIndexBuffer> CBufferManager::CreateIndexBuffer(const void* data, const uint32 dataSize)
	{
		return std::make_unique<CVulkanIndexBuffer>(m_context, dataSize, data);
	}

	std::unique_ptr<CVulkanUniformBuffer> CBufferManager::CreateUniformBuffer(const uint32 dataSize)
	{
		return std::make_unique<CVulkanUniformBuffer>(m_context, dataSize);
	}
}