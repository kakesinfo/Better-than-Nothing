/*
 *	Better than Nothing
 *	Copyright (C) 2026 kakesinfo
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

namespace BetterThanNothing::RenderingApi
{
	CommandBuffer::CommandBuffer(const CommandBufferCreateInfo& createInfo) : m_createInfo(createInfo)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_createInfo.commandPool->Handle();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(m_createInfo.device->Handle(), &allocInfo, &m_commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate command buffers!");
	}

	CommandBuffer::~CommandBuffer()
	{
		if (m_commandBuffer != VK_NULL_HANDLE)
			vkFreeCommandBuffers(m_createInfo.device->Handle(), m_createInfo.commandPool->Handle(), 1, &m_commandBuffer);
	}

	std::unique_ptr<CommandBuffer> CommandBuffer::Create(const CommandBufferCreateInfo& createInfo)
	{
		return std::make_unique<CommandBuffer>(createInfo);
	}

	void CommandBuffer::BeginRecording(const VkCommandBufferUsageFlags flags)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = flags;

		if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("failed to begin recording command buffer!");
	}

	void CommandBuffer::EndRecording()
	{
		if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to record command buffer!");
	}

	void CommandBuffer::BeginRendering(const VkRenderingInfo& renderingInfo) const
	{
		vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
	}

	void CommandBuffer::EndRendering() const
	{
		vkCmdEndRendering(m_commandBuffer);
	}

	void CommandBuffer::ExecuteRenderPass(const std::function<void(CommandBuffer*)>& callback, const VkRenderingInfo& renderingInfo)
	{
		BeginRendering(renderingInfo);
		callback(this);
		EndRendering();
	}

	void CommandBuffer::Reset()
	{
		if (vkResetCommandBuffer(m_commandBuffer, 0) != VK_SUCCESS)
			throw std::runtime_error("failed to reset command buffer!");
	}

	void CommandBuffer::SetViewport(VkExtent2D extent, const float minDepth, const float maxDepth)
	{
		SetViewport(0, 0, extent, minDepth, maxDepth);
	}

	void CommandBuffer::SetViewport(const float x, const float y, VkExtent2D extent, const float minDepth, const float maxDepth)
	{
		SetViewport(x, y, static_cast<float>(extent.width), static_cast<float>(extent.height), minDepth, maxDepth);
	}

	void CommandBuffer::SetViewport(const float x, const float y, const float width, const float height, const float minDepth, const float maxDepth)
	{
		const VkViewport viewport = {.x = x, .y = y, .width = width, .height = height, .minDepth = minDepth, .maxDepth = maxDepth};
		vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);
	}

	void CommandBuffer::SetScissor(VkExtent2D extent)
	{
		SetScissor(0, 0, extent);
	}

	void CommandBuffer::SetScissor(const int32_t offsetX, const int32_t offsetY, VkExtent2D extent)
	{
		const VkRect2D scissor = {.offset = {offsetX, offsetY}, .extent = extent};
		vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
	}

	void CommandBuffer::BindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint)
	{
		vkCmdBindPipeline(m_commandBuffer, bindPoint, pipeline);
	}

	void CommandBuffer::BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* buffers, const VkDeviceSize* offsets)
	{
		vkCmdBindVertexBuffers(m_commandBuffer, firstBinding, bindingCount, buffers, offsets);
	}

	void CommandBuffer::BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
	{
		vkCmdBindIndexBuffer(m_commandBuffer, buffer, offset, indexType);
	}

	void CommandBuffer::BindDescriptorSets(const VkPipelineLayout layout, const uint32_t firstSet, const uint32_t descriptorSetCount, const VkDescriptorSet* descriptorSets, const uint32_t dynamicOffsetCount, const uint32_t* dynamicOffsets)
	{
		BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, layout, firstSet, descriptorSetCount, descriptorSets, dynamicOffsetCount, dynamicOffsets);
	}

	void CommandBuffer::BindDescriptorSets(const VkPipelineBindPoint pipelineBindPoint, const VkPipelineLayout layout, const uint32_t firstSet, const uint32_t descriptorSetCount, const VkDescriptorSet* descriptorSets, uint32_t dynamicOffsetCount, const uint32_t* dynamicOffsets)
	{
		vkCmdBindDescriptorSets(m_commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, descriptorSets, dynamicOffsetCount, dynamicOffsets);
	}

	void CommandBuffer::Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance)
	{
		vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CommandBuffer::DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const int32_t vertexOffset, const uint32_t firstInstance)
	{
		vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void CommandBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const uint64_t bufferSize, const uint64_t offset)
	{
		VkBufferCopy copyRegion{};
		copyRegion.size = bufferSize;
		copyRegion.srcOffset = offset;

		vkCmdCopyBuffer(m_commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	}

	static void ResolveLayoutSync2(const VkImageLayout layout, VkPipelineStageFlags2& stageMask, VkAccessFlags2& accessMask)
	{
		switch (layout)
		{
			case VK_IMAGE_LAYOUT_UNDEFINED:
				stageMask = VK_PIPELINE_STAGE_2_NONE;
				accessMask = VK_ACCESS_2_NONE;
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				stageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
				accessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				stageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
				accessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
				break;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				stageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
				accessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
				break;
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
				accessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				stageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
				accessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
				stageMask = VK_PIPELINE_STAGE_2_NONE;
				accessMask = VK_ACCESS_2_NONE;
				break;
			default:
				stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				accessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
				break;
		}
	}

	void CommandBuffer::TransitionImageLayout2(VkImage image, const VkImageAspectFlags aspectMask, const uint32_t mipLevels, const VkImageLayout oldLayout, const VkImageLayout newLayout) const
	{
		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange = {aspectMask, 0, mipLevels, 0, 1};

		ResolveLayoutSync2(oldLayout, barrier.srcStageMask, barrier.srcAccessMask);
		ResolveLayoutSync2(newLayout, barrier.dstStageMask, barrier.dstAccessMask);

		// For color->color (e.g. Forward resolve -> ImGui overlay), keep explicit write->read|write dependency
		if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		}

		VkDependencyInfo depInfo{};
		depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		depInfo.imageMemoryBarrierCount = 1;
		depInfo.pImageMemoryBarriers = &barrier;

		PipelineBarrier2(depInfo);
	}

	void CommandBuffer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
	{
		VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT)
				aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		TransitionImageLayout2(image, aspectMask, mipLevels, oldLayout, newLayout);
	}

	void CommandBuffer::CopyBufferToImage(VkBuffer buffer, VkImage image, const uint32_t width, const uint32_t height, const uint64_t offset)
	{
		VkBufferImageCopy region{};
		region.bufferOffset = offset;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = {0, 0, 0};
		region.imageExtent = {width, height, 1};

		vkCmdCopyBufferToImage(m_commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}

	void CommandBuffer::PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* memoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* bufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* imageMemoryBarriers) const
	{
		vkCmdPipelineBarrier(m_commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, memoryBarriers, bufferMemoryBarrierCount, bufferMemoryBarriers, imageMemoryBarrierCount, imageMemoryBarriers);
	}

	void CommandBuffer::PipelineBarrier2(const VkDependencyInfo& dependencyInfo) const
	{
		vkCmdPipelineBarrier2(m_commandBuffer, &dependencyInfo);
	}

	void CommandBuffer::BlitImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, const VkImageBlit* blit, VkFilter filter)
	{
		vkCmdBlitImage(m_commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, 1, blit, filter);
	}

	void CommandBuffer::PushConstants(VkPipelineLayout layout, VkShaderStageFlags stageFlags, const uint32_t offset, const uint32_t size, const void* values)
	{
		vkCmdPushConstants(m_commandBuffer, layout, stageFlags, offset, size, values);
	}

	void CommandBuffer::SingleTimeCommands(const std::function<void(std::unique_ptr<CommandBuffer>&)>& callback, Device* device, CommandPool* commandPool, Queue* graphicsQueue)
	{
		CommandBufferCreateInfo createInfo{};
		createInfo.device = device;
		createInfo.commandPool = commandPool;

		auto commandBuffer = std::make_unique<CommandBuffer>(createInfo);

		commandBuffer->BeginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		callback(commandBuffer);
		commandBuffer->EndRecording();

		const auto vkCommandBuffer = commandBuffer->Handle();

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &vkCommandBuffer;

		const auto fence = Fence::Create({device, false});
		if (vkQueueSubmit(graphicsQueue->Handle(), 1, &submitInfo, fence->Handle()) != VK_SUCCESS)
			throw std::runtime_error("failed to submit single-time command buffer!");

		fence->Wait();
	}

	VkCommandBuffer CommandBuffer::Handle() const
	{
		return m_commandBuffer;
	}
}
