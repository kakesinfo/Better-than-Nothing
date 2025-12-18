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

namespace BetterThanNothing
{
	class CVulkanRenderPassContext;
	struct VulkanContext;

	class CVulkanCommandBuffer
	{
	public:
		explicit CVulkanCommandBuffer(VkCommandBuffer commandBuffer);
		~CVulkanCommandBuffer() = default;

		CVulkanCommandBuffer(const CVulkanCommandBuffer&) = delete;
		CVulkanCommandBuffer& operator=(const CVulkanCommandBuffer&) = delete;
		CVulkanCommandBuffer(CVulkanCommandBuffer&& other) = delete;
		CVulkanCommandBuffer& operator=(CVulkanCommandBuffer&& other) = delete;

		void Reset() const;
		void BeginRecording(VkCommandBufferUsageFlags flags = 0);
		void EndRecording();

		std::unique_ptr<CVulkanRenderPassContext> BeginRenderPass(VkRenderPassBeginInfo* pRenderPassInfo, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE);
		void EndRenderPass();

		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, std::uint32_t bufferSize) const;
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, std::uint32_t mipLevels) const;
		void CopyBufferToImage(VkBuffer buffer, VkImage image, std::uint32_t width, std::uint32_t height) const;

		void PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
			VkDependencyFlags dependencyFlags,
			std::uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
			std::uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
			std::uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const;

		void BlitImage(VkImage srcImage, VkImageLayout srcImageLayout,
			VkImage dstImage, VkImageLayout dstImageLayout,
			const VkImageBlit* pBlit, VkFilter filter) const;

		static void SingleTimeCommands(const std::function<void(std::unique_ptr<CVulkanCommandBuffer>&)>& callback, VulkanContext* context);

	private:
		static bool _hasStencilComponent(VkFormat format);

	public:
		[[nodiscard]]
		VkCommandBuffer Handle() const;

	private:
		VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;

		bool m_bRecording = false;
		bool m_bRenderPassActive = false;
	};

}
