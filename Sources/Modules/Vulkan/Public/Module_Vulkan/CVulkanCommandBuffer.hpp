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
