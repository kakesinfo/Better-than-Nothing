#include "CApplicationContext.hpp"
#include "CVulkanCommandBuffer.hpp"

namespace BetterThanNothing
{
	CVulkanCommandBuffer::CVulkanCommandBuffer(VkCommandBuffer commandBuffer)
		: m_commandBuffer(commandBuffer)
	{
	}

	void CVulkanCommandBuffer::Reset()
	{
		if (m_bRecording)
			throw std::runtime_error("Command buffer is recording");

		if (vkResetCommandBuffer(m_commandBuffer, 0) != VK_SUCCESS)
			throw std::runtime_error("failed to reset command buffer!");
	}

	void CVulkanCommandBuffer::BeginRecording(const VkCommandBufferUsageFlags flags)
	{
		if (m_bRecording)
			throw std::runtime_error("Command buffer is already recording");

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = flags;

		if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("failed to begin recording command buffer!");

		m_bRecording = true;
	}

	void CVulkanCommandBuffer::EndRecording()
	{
		if (!m_bRecording)
			throw std::runtime_error("Command buffer is not recording");

		if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to record command buffer!");

		m_bRecording = false;
	}

	void CVulkanCommandBuffer::CmdBeginRenderPass(VkRenderPassBeginInfo* renderPassInfo, VkSubpassContents subpassContents) const
	{
		vkCmdBeginRenderPass(m_commandBuffer, renderPassInfo, subpassContents);
	}

	void CVulkanCommandBuffer::CmdEndRenderPass() const
	{
		vkCmdEndRenderPass(m_commandBuffer);
	}

	void CVulkanCommandBuffer::CmdSetViewport(const VkViewport* viewports, uint32 firstViewport, uint32 viewportCount) const
	{
		vkCmdSetViewport(m_commandBuffer, firstViewport, viewportCount, viewports);
	}

	void CVulkanCommandBuffer::CmdSetScissor(const VkRect2D* scissors, uint32 firstScissor, uint32 scissorCount) const
	{
		vkCmdSetScissor(m_commandBuffer, firstScissor, scissorCount, scissors);
	}

	void CVulkanCommandBuffer::CmdBindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint) const
	{
		vkCmdBindPipeline(m_commandBuffer, bindPoint, pipeline);
	}

	void CVulkanCommandBuffer::CmdBindVertexBuffers(uint32 firstBinding, uint32 bindingCount, const VkBuffer* buffers, const VkDeviceSize* offsets) const
	{
		vkCmdBindVertexBuffers(m_commandBuffer, firstBinding, bindingCount, buffers, offsets);
	}

	void CVulkanCommandBuffer::CmdBindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) const
	{
		vkCmdBindIndexBuffer(m_commandBuffer, buffer, offset, indexType);
	}

	void CVulkanCommandBuffer::CmdDraw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) const
	{
		vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CVulkanCommandBuffer::CmdDrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, int32 vertexOffset, uint32 firstInstance) const
	{
		vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void CVulkanCommandBuffer::CmdCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const uint32 bufferSize) const
	{
		VkBufferCopy copyRegion{};
		copyRegion.size = bufferSize;

		vkCmdCopyBuffer(m_commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	}

	void CVulkanCommandBuffer::SingleTimeCommands(const std::function<void(std::unique_ptr<CVulkanCommandBuffer>&)>& callback, CApplicationContext* context)
	{
		const auto device = context->GetVulkanDevice()->LogicalHandle();
		const auto commandPool = context->GetRenderer()->GetVulkanCommandPool()->Handle();
		const auto graphicsQueue = context->GetGraphicsQueue()->Handle();

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		auto pCommandBuffer = std::make_unique<CVulkanCommandBuffer>(commandBuffer);

		pCommandBuffer->BeginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		callback(pCommandBuffer);
		pCommandBuffer->EndRecording();

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}


	VkCommandBuffer CVulkanCommandBuffer::Handle() const
	{
		return m_commandBuffer;
	}
}