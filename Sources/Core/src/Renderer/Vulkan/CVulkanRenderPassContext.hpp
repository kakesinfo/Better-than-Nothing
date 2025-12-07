#pragma once

namespace BetterThanNothing
{
	class CVulkanRenderPassContext
	{
	public:
		explicit CVulkanRenderPassContext(VkCommandBuffer commandBuffer);
		~CVulkanRenderPassContext() = default;

		CVulkanRenderPassContext(const CVulkanRenderPassContext&) = delete;
		CVulkanRenderPassContext& operator=(const CVulkanRenderPassContext&) = delete;
		CVulkanRenderPassContext(CVulkanRenderPassContext&& other) = delete;
		CVulkanRenderPassContext& operator=(CVulkanRenderPassContext&& other) = delete;

		void SetViewport(const VkViewport* pViewports, uint32 firstViewport = 0, uint32 viewportCount = 1) const;
		void SetScissor(const VkRect2D* pScissors, uint32 firstScissor = 0, uint32 scissorCount = 1) const;
		void BindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const;
		void BindVertexBuffers(uint32 firstBinding, uint32 bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) const;
		void BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) const;

		void BindDescriptorSets(
			VkPipelineLayout layout,
			uint32_t firstSet,
			uint32_t descriptorSetCount,
			const VkDescriptorSet* pDescriptorSets,
			uint32_t dynamicOffsetCount = 0,
			const uint32_t* pDynamicOffsets = nullptr
		) const;

		void BindDescriptorSets(
			VkPipelineBindPoint pipelineBindPoint,
			VkPipelineLayout layout,
			uint32_t firstSet,
			uint32_t descriptorSetCount,
			const VkDescriptorSet* pDescriptorSets,
			uint32_t dynamicOffsetCount = 0,
			const uint32_t* pDynamicOffsets = nullptr
		) const;


		void Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) const;
		void DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, int32 vertexOffset, uint32 firstInstance) const;

	private:
		VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
	};
}