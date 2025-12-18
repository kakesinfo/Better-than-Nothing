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

	CVulkanRenderPassContext::CVulkanRenderPassContext(VkCommandBuffer commandBuffer)
		: m_commandBuffer(commandBuffer)
	{
	}

	void CVulkanRenderPassContext::SetViewport(const VkViewport* pViewports, std::uint32_t firstViewport, std::uint32_t viewportCount) const
	{
		vkCmdSetViewport(m_commandBuffer, firstViewport, viewportCount, pViewports);
	}

	void CVulkanRenderPassContext::SetScissor(const VkRect2D* pScissors, std::uint32_t firstScissor, std::uint32_t scissorCount) const
	{
		vkCmdSetScissor(m_commandBuffer, firstScissor, scissorCount, pScissors);
	}

	void CVulkanRenderPassContext::BindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint) const
	{
		vkCmdBindPipeline(m_commandBuffer, bindPoint, pipeline);
	}

	void CVulkanRenderPassContext::BindVertexBuffers(std::uint32_t firstBinding, std::uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) const
	{
		vkCmdBindVertexBuffers(m_commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
	}

	void CVulkanRenderPassContext::BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) const
	{
		vkCmdBindIndexBuffer(m_commandBuffer, buffer, offset, indexType);
	}

	void CVulkanRenderPassContext::BindDescriptorSets(const VkPipelineLayout layout, const std::uint32_t firstSet, const std::uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, const std::uint32_t dynamicOffsetCount, const std::uint32_t* pDynamicOffsets) const {
		BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
	}

	void CVulkanRenderPassContext::BindDescriptorSets(const VkPipelineBindPoint pipelineBindPoint, const VkPipelineLayout layout, const std::uint32_t firstSet, const std::uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, std::uint32_t dynamicOffsetCount, const std::uint32_t* pDynamicOffsets) const {
		vkCmdBindDescriptorSets(m_commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
	}

	void CVulkanRenderPassContext::Draw(const std::uint32_t vertexCount, const std::uint32_t instanceCount, const std::uint32_t firstVertex, const std::uint32_t firstInstance) const
	{
		vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CVulkanRenderPassContext::DrawIndexed(const std::uint32_t indexCount, const std::uint32_t instanceCount, const std::uint32_t firstIndex, const std::int32_t vertexOffset, const std::uint32_t firstInstance) const
	{
		vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}
};