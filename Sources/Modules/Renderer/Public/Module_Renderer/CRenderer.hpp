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
	class CWindow;

	class CVulkanSwapChain;
	class CVulkanRenderPass;
	class CVulkanCommandPool;
	class CVulkanCommandBuffer;
	class CVulkanPipeline;
	class CVulkanPipelineBuilder;
	class CVulkanSemaphore;
	class CVulkanFence;

	class CRenderList;
	class CRenderCache;

	class CGlobalUniformProvider;
	class CDynamicUniformProvider;
	class CBufferManager;
	class CCamera;

	struct VulkanContext;
	struct VertexData;
	struct VulkanFrame;
	struct CameraData;
	struct DirectionalLightData;

	class CRenderer final
	{
	public:
		explicit CRenderer(VulkanContext* pVkContext);
		~CRenderer();

		void RenderFrame(CRenderList* renderList, const CameraData& camera, const DirectionalLightData& directionalLight);

	private:
		void _createCommandBuffers() const;
		void _createSyncObjects();
		void _createDescriptorSetLayout();
		void _createDescriptorPool();
		void _createGlobalDescriptorSets();
		void _createDynamicDescriptorSets();

		void _recordCommandBuffer(const CRenderList* renderList, const CameraData& camera, const DirectionalLightData& directionalLight, std::uint32_t imageIndex) const;

		static VkVertexInputBindingDescription s_getVertexBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> s_getVertexAttributeDescriptions();

	public:
		void AddPipeline(const std::function<void(CVulkanPipelineBuilder*)> &callback, CRenderCache* pRenderCache);

	private:
		VulkanContext* m_pVkContext = nullptr;

		std::vector<VulkanFrame> m_frames;
		std::uint32_t m_currentFrame = 0;

		std::unique_ptr<CBufferManager> m_pBufferFactory = nullptr;

		std::unique_ptr<CGlobalUniformProvider> m_pGlobalUniformProvider = nullptr;
		std::unique_ptr<CDynamicUniformProvider> m_pDynamicUniformProvider = nullptr;
	};
}
