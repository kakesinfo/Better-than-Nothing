#include "CWindowBuilder.hpp"
#include "CVulkanCommandBuffer.hpp"
#include "CVulkanRenderPassContext.hpp"

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

		if (m_bRenderPassActive)
			throw std::runtime_error("Trying to end recording while render pass is active");

		if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to record command buffer!");

		m_bRecording = false;
	}

	std::unique_ptr<CVulkanRenderPassContext> CVulkanCommandBuffer::BeginRenderPass(VkRenderPassBeginInfo* pRenderPassInfo, VkSubpassContents subpassContents)
	{
		if (m_bRenderPassActive)
			throw std::runtime_error("Trying to begin render pass without ending the previous one");

		m_bRenderPassActive = true;

		vkCmdBeginRenderPass(m_commandBuffer, pRenderPassInfo, subpassContents);

		return std::make_unique<CVulkanRenderPassContext>(m_commandBuffer);
	}

	void CVulkanCommandBuffer::EndRenderPass()
	{
		if (!m_bRenderPassActive)
			throw std::runtime_error("Trying to end render pass without having begun one");

		m_bRenderPassActive = false;

		vkCmdEndRenderPass(m_commandBuffer);
	}

	void CVulkanCommandBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32 bufferSize) const
	{
		VkBufferCopy copyRegion{};
		copyRegion.size = bufferSize;

		vkCmdCopyBuffer(m_commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	}

	void CVulkanCommandBuffer::SingleTimeCommands(const std::function<void(std::unique_ptr<CVulkanCommandBuffer>&)>& callback, VulkanContext* context)
	{
		const auto device = context->vkDevice->Handle();
		const auto commandPool = context->vkCommandPool->Handle();
		const auto graphicsQueue = context->vkGraphicsQueue->Handle();

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