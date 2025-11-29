//
// Created by acoezard on 11/25/24.
//

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanContext* CVulkanBufferFactory::m_context = nullptr;

	void CVulkanBufferFactory::Initialize(CVulkanContext* context)
	{
		m_context = context;
	}

	void CVulkanBufferFactory::Shutdown()
	{
		m_context = nullptr;
	}

	CVulkanBuffer CVulkanBufferFactory::CreateVertexBuffer(const void* data, const uint32 dataSize)
	{
		const CVulkanBuffer stagingBuffer = {dataSize, EVulkanBufferType::STAGING, m_context};
		CVulkanBuffer vertexBuffer = {dataSize, EVulkanBufferType::VERTEX, m_context};

		void* stagingData;
		stagingBuffer.MapMemory(&stagingData);
		memcpy(stagingData, data, dataSize);
		stagingBuffer.UnmapMemory();

		CVulkanCommandBuffer::SingleTimeCommands([&](const std::unique_ptr<CVulkanCommandBuffer>& commandBuffer) {
			commandBuffer->CopyBuffer(stagingBuffer.Handle(), vertexBuffer.Handle(), dataSize);
		}, m_context);

		return vertexBuffer;
	}

	CVulkanBuffer CVulkanBufferFactory::CreateIndexBuffer(const void* data, const uint32 dataSize)
	{
		const CVulkanBuffer stagingBuffer = {dataSize, EVulkanBufferType::STAGING, m_context};
		CVulkanBuffer indexBuffer = {dataSize, EVulkanBufferType::INDEX, m_context};

		void* stagingData;
		stagingBuffer.MapMemory(&stagingData);
		memcpy(stagingData, data, dataSize);
		stagingBuffer.UnmapMemory();

		CVulkanCommandBuffer::SingleTimeCommands([&](const std::unique_ptr<CVulkanCommandBuffer>& commandBuffer) {
			commandBuffer->CopyBuffer(stagingBuffer.Handle(), indexBuffer.Handle(), dataSize);
		}, m_context);

		return indexBuffer;
	}
}