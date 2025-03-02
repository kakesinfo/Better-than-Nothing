#pragma once

namespace BetterThanNothing
{
	class CApplicationContext;

	class CVulkanCommandBuffer
	{
	private:
		VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;

		bool m_bRecording = false;

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

		void CmdBeginRenderPass(VkRenderPassBeginInfo* pRenderPassInfo, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const;
		void CmdEndRenderPass() const;
		void CmdSetViewport(const VkViewport* pViewports, uint32 firstViewport = 0, uint32 viewportCount = 1) const;
		void CmdSetScissor(const VkRect2D* pScissors, uint32 firstScissor = 0, uint32 scissorCount = 1) const;
		void CmdBindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const;
		void CmdBindVertexBuffers(uint32 firstBinding, uint32 bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) const;
		void CmdBindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) const;
		void CmdDraw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) const;
		void CmdDrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, int32 vertexOffset, uint32 firstInstance) const;

		void CmdCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32 bufferSize) const;

		static void SingleTimeCommands(const std::function<void(std::unique_ptr<CVulkanCommandBuffer>&)>& callback, CApplicationContext* context);

		[[nodiscard]]
		VkCommandBuffer Handle() const;
	};

}
