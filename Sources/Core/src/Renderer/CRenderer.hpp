#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class CWindow;

	class CVulkanSwapChain;
	class CVulkanRenderPass;
	class CVulkanCommandPool;
	class CVulkanCommandBuffer;
	class CVulkanPipeline;
	class CVulkanSemaphore;
	class CVulkanFence;

	class CRenderList;
	class CRenderCache;

	class CGlobalUniformProvider;
	class CDynamicUniformProvider;

	struct VertexData;

	class CRenderer final : public IRenderer
	{
	public:
		explicit CRenderer(VulkanContext& vkContext);
		~CRenderer() override;

		void RenderFrame(CRenderList* renderList);

	private:
		void _createCommandBuffers();
		void _createSyncObjects();
		void _createDescriptorSetLayout();
		void _createDescriptorPool();
		void _createGlobalDescriptorSets();
		void _createDynamicDescriptorSets();

		void _recordCommandBuffer(CRenderList* renderList, uint32 imageIndex) const;

	public:
		void AddPipeline(const std::function<void(IPipelineBuilder *)> &callback) override;

	private:
		VulkanContext m_vkContext{};

		std::vector<VulkanFrame> m_frames;
		uint32 m_currentFrame = 0;

		std::unique_ptr<CBufferManager> m_pBufferFactory = nullptr;
		std::unique_ptr<CRenderCache> m_pRenderCache = nullptr;

		std::unique_ptr<CGlobalUniformProvider> m_pGlobalUniformProvider = nullptr;
		std::unique_ptr<CDynamicUniformProvider> m_pDynamicUniformProvider = nullptr;
	};
}
