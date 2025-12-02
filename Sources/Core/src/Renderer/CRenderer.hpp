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

	struct CVertex;

	class CRenderer : public IRenderer
	{
	private:
		CVulkanContext m_vkContext{};

		std::vector<CVulkanFrame> m_frames;
		uint32 m_currentFrame = 0;

		std::unique_ptr<CBufferManager> m_pBufferFactory = nullptr;
		std::unique_ptr<CShaderPool> m_pShaderPool = nullptr;

		std::vector<CMesh*> m_renderQueue;

	public:
		explicit CRenderer(CVulkanContext& vkContext);
		~CRenderer() override;

		void RenderFrame();
    	void Render(CMesh* pMesh) override;

	private:
		void _createCommandBuffers();
		void _createSyncObjects();
		void _createDescriptorSetLayout();
		void _createDescriptorPool();
		void _createDescriptorSets();
		void _createUniformBuffers();

		void _updateUniformBuffer() const;

		void _recordCommandBuffer(uint32 imageIndex);

	public:
		void AddPipeline(const std::function<void(IPipelineBuilder *)> &callback) override;
	};
}
