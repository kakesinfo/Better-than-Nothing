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
	class VulkanCommandBufferContext;
	struct VulkanContext;

	class VulkanCommandBuffer
	{
	public:
		explicit VulkanCommandBuffer(VkCommandBuffer commandBuffer);
		~VulkanCommandBuffer() = default;

		VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
		VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = delete;
		VulkanCommandBuffer(VulkanCommandBuffer&& other) = delete;
		VulkanCommandBuffer& operator=(VulkanCommandBuffer&& other) = delete;

		void					   Reset() const;
		VulkanCommandBufferContext BeginRecording(VkCommandBufferUsageFlags flags = 0);
		void					   EndRecording();

		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, std::uint32_t bufferSize) const;
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
								   std::uint32_t mipLevels) const;
		void CopyBufferToImage(VkBuffer buffer, VkImage image, std::uint32_t width, std::uint32_t height) const;

		void PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
							 VkDependencyFlags dependencyFlags, std::uint32_t memoryBarrierCount,
							 const VkMemoryBarrier* memoryBarriers, std::uint32_t bufferMemoryBarrierCount,
							 const VkBufferMemoryBarrier* bufferMemoryBarriers, std::uint32_t imageMemoryBarrierCount,
							 const VkImageMemoryBarrier* imageMemoryBarriers) const;

		void BlitImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
					   const VkImageBlit* blit, VkFilter filter) const;

		static void SingleTimeCommands(const std::function<void(std::unique_ptr<VulkanCommandBuffer>&)>& callback,
									   VulkanContext*													 context);

	private:
		static bool HasStencilComponent(VkFormat format);

	public:
		[[nodiscard]]
		VkCommandBuffer Handle() const;

	private:
		VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;

		bool m_isRecording = false;
		// bool m_isRenderPassActive = false;
	};

}
