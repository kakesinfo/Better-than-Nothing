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

		void SetViewport(const VkViewport* pViewports, std::uint32_t firstViewport = 0, std::uint32_t viewportCount = 1) const;
		void SetScissor(const VkRect2D* pScissors, std::uint32_t firstScissor = 0, std::uint32_t scissorCount = 1) const;
		void BindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const;
		void BindVertexBuffers(std::uint32_t firstBinding, std::uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) const;
		void BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) const;

		void BindDescriptorSets(
			VkPipelineLayout layout,
			std::uint32_t firstSet,
			std::uint32_t descriptorSetCount,
			const VkDescriptorSet* pDescriptorSets,
			std::uint32_t dynamicOffsetCount = 0,
			const std::uint32_t* pDynamicOffsets = nullptr
		) const;

		void BindDescriptorSets(
			VkPipelineBindPoint pipelineBindPoint,
			VkPipelineLayout layout,
			std::uint32_t firstSet,
			std::uint32_t descriptorSetCount,
			const VkDescriptorSet* pDescriptorSets,
			std::uint32_t dynamicOffsetCount = 0,
			const std::uint32_t* pDynamicOffsets = nullptr
		) const;


		void Draw(std::uint32_t vertexCount, std::uint32_t instanceCount, std::uint32_t firstVertex, std::uint32_t firstInstance) const;
		void DrawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance) const;

	private:
		VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
	};
}