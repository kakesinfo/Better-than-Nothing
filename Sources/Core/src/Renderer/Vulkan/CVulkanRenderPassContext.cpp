#include "CVulkanRenderPassContext.hpp"

namespace BetterThanNothing
{

	CVulkanRenderPassContext::CVulkanRenderPassContext(VkCommandBuffer commandBuffer)
		: m_commandBuffer(commandBuffer)
	{
	}

	void CVulkanRenderPassContext::SetViewport(const VkViewport* pViewports, uint32 firstViewport, uint32 viewportCount) const
	{
		vkCmdSetViewport(m_commandBuffer, firstViewport, viewportCount, pViewports);
	}

	void CVulkanRenderPassContext::SetScissor(const VkRect2D* pScissors, uint32 firstScissor, uint32 scissorCount) const
	{
		vkCmdSetScissor(m_commandBuffer, firstScissor, scissorCount, pScissors);
	}

	void CVulkanRenderPassContext::BindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint) const
	{
		vkCmdBindPipeline(m_commandBuffer, bindPoint, pipeline);
	}

	void CVulkanRenderPassContext::BindVertexBuffers(uint32 firstBinding, uint32 bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) const
	{
		vkCmdBindVertexBuffers(m_commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
	}

	void CVulkanRenderPassContext::BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) const
	{
		vkCmdBindIndexBuffer(m_commandBuffer, buffer, offset, indexType);
	}

	void CVulkanRenderPassContext::BindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) const {
		vkCmdBindDescriptorSets(m_commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
	}

	void CVulkanRenderPassContext::Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) const
	{
		vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CVulkanRenderPassContext::DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, int32 vertexOffset, uint32 firstInstance) const
	{
		vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}
};