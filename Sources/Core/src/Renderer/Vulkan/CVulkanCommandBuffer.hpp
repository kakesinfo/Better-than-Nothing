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

		static void SingleTimeCommands(const std::function<void(std::unique_ptr<CVulkanCommandBuffer>&)>& callback, VulkanContext* context);

		[[nodiscard]]
		VkCommandBuffer Handle() const;

	private:
		VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;

		bool m_bRecording = false;
		bool m_bRenderPassActive = false;
	};

}
