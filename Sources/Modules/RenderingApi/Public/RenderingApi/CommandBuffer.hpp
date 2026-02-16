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

#pragma once

#include <functional>
#include <memory>

namespace BetterThanNothing
{
	class Device;
	class CommandPool;
	class Queue;

	struct CommandBufferCreateInfo
	{
		Device*		 device;
		CommandPool* commandPool;
	};

	class CommandBuffer
	{
	public:
		explicit CommandBuffer(const CommandBufferCreateInfo& createInfo);
		~CommandBuffer();

		[[nodiscard]] static std::unique_ptr<CommandBuffer> Create(const CommandBufferCreateInfo& createInfo);

		void BeginRecording(VkCommandBufferUsageFlags flags = 0);
		void EndRecording();

		void ExecuteRenderPass(const std::function<void(CommandBuffer*)>& callback, const VkRenderingInfo& renderingInfo);

		void Reset();

		void SetViewport(VkExtent2D extent, float minDepth, float maxDepth);
		void SetViewport(float x, float y, VkExtent2D extent, float minDepth, float maxDepth);
		void SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth);

		void SetScissor(VkExtent2D extent);
		void SetScissor(int32_t offsetX, int32_t offsetY, VkExtent2D extent);

		void BindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint);
		void BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* buffers, const VkDeviceSize* offsets);
		void BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
		void BindDescriptorSets(const VkPipelineLayout layout, const uint32_t firstSet, const uint32_t descriptorSetCount, const VkDescriptorSet* descriptorSets, const uint32_t dynamicOffsetCount, const uint32_t* dynamicOffsets);
		void BindDescriptorSets(const VkPipelineBindPoint pipelineBindPoint, const VkPipelineLayout layout, const uint32_t firstSet, const uint32_t descriptorSetCount, const VkDescriptorSet* descriptorSets, uint32_t dynamicOffsetCount, const uint32_t* dynamicOffsets);

		void Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance);
		void DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const int32_t vertexOffset, const uint32_t firstInstance);

		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t bufferSize);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* memoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* bufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* imageMemoryBarriers) const;
		void BlitImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, const VkImageBlit* blit, VkFilter filter);
		void PushConstants(VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* values);

		static void SingleTimeCommands(const std::function<void(std::unique_ptr<CommandBuffer>&)>& callback, Device* device, CommandPool* commandPool, Queue* graphicsQueue);

		[[nodiscard]] VkCommandBuffer Handle() const;

	private:
		CommandBufferCreateInfo m_createInfo;
		VkCommandBuffer			m_commandBuffer = VK_NULL_HANDLE;
	};
}
