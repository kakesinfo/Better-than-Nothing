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

	void CommandBuffer::ExecuteRenderPass(const std::function<void(CommandBuffer*)>& callback, const VkRenderingInfo& renderingInfo)
	{
		vkCmdBeginRendering(m_commandBuffer, &renderingInfo);

		callback(this);

		vkCmdEndRendering(m_commandBuffer);
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
		const VkViewport viewport = { .x = x, .y = y, .width = width, .height = height, .minDepth = minDepth, .maxDepth = maxDepth };
		vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);
	}

	void CommandBuffer::SetScissor(VkExtent2D extent)
	{
		SetScissor(0, 0, extent);
	}

	void CommandBuffer::SetScissor(const std::int32_t offsetX, const std::int32_t offsetY, VkExtent2D extent)
	{
		const VkRect2D scissor = { .offset = {offsetX, offsetY}, .extent = extent };
		vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
	}

	void CommandBuffer::BindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint)
	{
		vkCmdBindPipeline(m_commandBuffer, bindPoint, pipeline);
	}

	void CommandBuffer::BindVertexBuffers(std::uint32_t firstBinding, std::uint32_t bindingCount,
													   const VkBuffer* buffers, const VkDeviceSize* offsets)
	{
		vkCmdBindVertexBuffers(m_commandBuffer, firstBinding, bindingCount, buffers, offsets);
	}

	void CommandBuffer::BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
	{
		vkCmdBindIndexBuffer(m_commandBuffer, buffer, offset, indexType);
	}

	void CommandBuffer::BindDescriptorSets(const VkPipelineLayout layout, const std::uint32_t firstSet,
														const std::uint32_t	   descriptorSetCount,
														const VkDescriptorSet* descriptorSets,
														const std::uint32_t	   dynamicOffsetCount,
														const std::uint32_t*   dynamicOffsets)
	{
		BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, layout, firstSet, descriptorSetCount, descriptorSets,
						   dynamicOffsetCount, dynamicOffsets);
	}

	void CommandBuffer::BindDescriptorSets(const VkPipelineBindPoint pipelineBindPoint,
														const VkPipelineLayout layout, const std::uint32_t firstSet,
														const std::uint32_t	   descriptorSetCount,
														const VkDescriptorSet* descriptorSets,
														std::uint32_t		   dynamicOffsetCount,
														const std::uint32_t*   dynamicOffsets)
	{
		vkCmdBindDescriptorSets(m_commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount,
								descriptorSets, dynamicOffsetCount, dynamicOffsets);
	}

	void CommandBuffer::Draw(const std::uint32_t vertexCount, const std::uint32_t instanceCount,
										  const std::uint32_t firstVertex, const std::uint32_t firstInstance)
	{
		vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CommandBuffer::DrawIndexed(const std::uint32_t indexCount, const std::uint32_t instanceCount,
												 const std::uint32_t firstIndex, const std::int32_t vertexOffset,
												 const std::uint32_t firstInstance)
	{
		vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void CommandBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, std::uint32_t bufferSize)
	{
		VkBufferCopy copyRegion{};
		copyRegion.size = bufferSize;

		vkCmdCopyBuffer(m_commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	}

	void CommandBuffer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, std::uint32_t mipLevels)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) // HasStencilComponent
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		else
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else
		{
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(m_commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	void CommandBuffer::CopyBufferToImage(VkBuffer buffer, VkImage image, std::uint32_t width, std::uint32_t height)
	{
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
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

	void CommandBuffer::PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, std::uint32_t memoryBarrierCount, const VkMemoryBarrier* memoryBarriers, std::uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* bufferMemoryBarriers, std::uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* imageMemoryBarriers) const
	{
		vkCmdPipelineBarrier(m_commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, memoryBarriers, bufferMemoryBarrierCount, bufferMemoryBarriers, imageMemoryBarrierCount, imageMemoryBarriers);
	}

	void CommandBuffer::BlitImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, const VkImageBlit* blit, VkFilter filter)
	{
		vkCmdBlitImage(m_commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, 1, blit, filter);
	}

	void CommandBuffer::PushConstants(VkPipelineLayout layout, VkShaderStageFlags stageFlags, const std::uint32_t offset, const std::uint32_t size, const void* values)
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

		vkQueueSubmit(graphicsQueue->Handle(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue->Handle());
	}

	VkCommandBuffer CommandBuffer::Handle() const
	{
		return m_commandBuffer;
	}
}
