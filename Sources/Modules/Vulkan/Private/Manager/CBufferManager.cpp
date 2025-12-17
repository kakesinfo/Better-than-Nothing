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