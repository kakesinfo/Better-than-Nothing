#pragma once
#include "VulkanContext.hpp"

namespace BetterThanNothing
{
	class CVulkanRenderPassContext;

	class CVulkanCommandBuffer
	{
	public:
		explicit CVulkanCommandBuffer(VkCommandBuffer commandBuffer);
		~CVulkanCommandBuffer() = default;

		CVulkanCommandBuffer(const CVulkanCommandBuffer&) = delete;
		CVulkanCommandBuffer& operator=(const CVulkanCommandBuffer&) = delete;
		CVulkanCommandBuffer(CVulkanCommandBuffer&& other) = delete;
		CVulkanCommandBuffer& operator=(CVulkanCommandBuffer&& other) = delete;

		void Reset();
		void BeginRecording(VkCommandBufferUsageFlags flags = 0);
		void EndRecording();

		std::unique_ptr<CVulkanRenderPassContext> BeginRenderPass(VkRenderPassBeginInfo* pRenderPassInfo, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE);
		void EndRenderPass();

		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32 bufferSize) const;
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32 mipLevels) const;
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32 width, uint32 height) const;

		void PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
			VkDependencyFlags dependencyFlags,
			uint32 memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
			uint32 bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
			uint32 imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const;

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
