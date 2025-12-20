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
	class VulkanRenderPassContext
	{
	public:
		explicit VulkanRenderPassContext(VkCommandBuffer commandBuffer);
		~VulkanRenderPassContext() = default;

		VulkanRenderPassContext(const VulkanRenderPassContext&) = delete;
		VulkanRenderPassContext& operator=(const VulkanRenderPassContext&) = delete;
		VulkanRenderPassContext(VulkanRenderPassContext&& other) = delete;
		VulkanRenderPassContext& operator=(VulkanRenderPassContext&& other) = delete;

		void SetViewport(const VkViewport* viewports, std::uint32_t firstViewport = 0, std::uint32_t viewportCount = 1) const;
		void SetScissor(const VkRect2D* scissors, std::uint32_t firstScissor = 0, std::uint32_t scissorCount = 1) const;
		void BindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const;
		void BindVertexBuffers(std::uint32_t firstBinding, std::uint32_t bindingCount, const VkBuffer* buffers, const VkDeviceSize* offsets) const;
		void BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) const;

		void BindDescriptorSets(VkPipelineLayout layout, std::uint32_t firstSet, std::uint32_t descriptorSetCount, const VkDescriptorSet* descriptorSets, std::uint32_t dynamicOffsetCount = 0,
								const std::uint32_t* dynamicOffsets = nullptr) const;

		void BindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, std::uint32_t firstSet, std::uint32_t descriptorSetCount, const VkDescriptorSet* descriptorSets, std::uint32_t dynamicOffsetCount = 0,
								const std::uint32_t* dynamicOffsets = nullptr) const;

		void Draw(std::uint32_t vertexCount, std::uint32_t instanceCount, std::uint32_t firstVertex, std::uint32_t firstInstance) const;
		void DrawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance) const;

	private:
		VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
	};
}