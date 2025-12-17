#pragma once
#include <functional>

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
