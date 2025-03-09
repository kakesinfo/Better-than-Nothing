#pragma once

namespace BetterThanNothing
{
	class CApplicationContext;

	class CVulkanSwapChain;
	class CVulkanRenderPass;
	class CVulkanCommandPool;
	class CVulkanCommandBuffer;
	class CVulkanPipeline;
	class CVulkanSemaphore;
	class CVulkanFence;

	struct CVertex;

	class CRenderer
	{
	private:
		CApplicationContext* m_context = nullptr;

		std::unique_ptr<CVulkanSwapChain> m_pVulkanSwapChain = nullptr;
		std::unique_ptr<CVulkanRenderPass> m_pVulkanRenderPass = nullptr;
		std::unique_ptr<CVulkanCommandPool> m_pVulkanCommandPool = nullptr;

		std::vector<std::unique_ptr<CVulkanCommandBuffer>> m_commandBuffers;

		std::unique_ptr<CVulkanPipeline> m_pPipeline = nullptr;

		std::unique_ptr<CVulkanBufferFactory> m_pBufferFactory = nullptr;

		std::vector<std::unique_ptr<CVulkanSemaphore>> m_imageAvailableSemaphores;
		std::vector<std::unique_ptr<CVulkanSemaphore>> m_renderFinishedSemaphores;
		std::vector<std::unique_ptr<CVulkanFence>> m_frameInFlightFences;

		uint32 m_currentFrame = 0;

		CVulkanBuffer m_vertexBuffer;
		CVulkanBuffer m_indexBuffer;

	public:
		explicit CRenderer(CApplicationContext* context): m_context(context) {}
		~CRenderer() = default;

		void Initialize();
		void Render();
		void AddPipeline(const std::function<void(VulkanPipelineBuilder&)>& callback);

	private:
		void _createCommandBuffers();
		void _createSyncObjects();
		void _recordCommandBuffer(uint32 imageIndex);

	public:
		std::unique_ptr<CVulkanSwapChain>& GetVulkanSwapChain();
		std::unique_ptr<CVulkanRenderPass>& GetVulkanRenderPass();
		std::unique_ptr<CVulkanCommandPool>& GetVulkanCommandPool();
	};
}
