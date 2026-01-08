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
	class Window;

	class VulkanSwapChain;
	class VulkanRenderPass;
	class VulkanCommandPool;
	class VulkanCommandBuffer;
	class VulkanPipeline;
	class VulkanPipelineBuilder;
	class VulkanSemaphore;
	class VulkanFence;

	class RenderList;
	class RenderCache;
	class RenderGraph;

	class GlobalUniformProvider;
	class DynamicUniformProvider;
	class BufferManager;
	class Camera;

	struct VulkanContext;
	struct VertexData;
	struct VulkanFrame;
	struct CameraData;
	struct DirectionalLightData;

	class ResourceManager;

	class LegacyRenderer final
	{
	public:
		explicit LegacyRenderer(VulkanContext* context, ResourceManager* resourceManager);
		~LegacyRenderer();

		void RenderFrame(const RenderList* renderList, const CameraData& camera);

	private:
		void CreateCommandBuffers() const;
		void CreateSyncObjects();
		void CreateDescriptorSetLayout();
		void CreateDescriptorPool();
		void CreateGlobalDescriptorSets();
		void CreateDynamicDescriptorSets();

		void CreatePointLightBuffers(std::uint32_t maxLights) const;
		void CreatePointLightDescriptorSets(std::uint32_t maxLights);

		void RecordCommandBuffer(const RenderList* renderList, const CameraData& camera,
								 std::uint32_t imageIndex) const;
		void DrawPointLights(const RenderList* renderList) const;

		static VkVertexInputBindingDescription				  GetVertexBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> GetVertexAttributeDescriptions();

	public:
		void AddPipeline(const std::function<void(VulkanPipelineBuilder*)>& callback, RenderCache* renderCache);

	private:
		VulkanContext*							m_context = nullptr;
		ResourceManager*						m_resourceManager = nullptr;
		std::vector<VulkanFrame>				m_frames;
		std::uint32_t							m_currentFrame = 0;
		std::unique_ptr<BufferManager>			m_bufferFactory = nullptr;
		std::unique_ptr<GlobalUniformProvider>	m_globalUniformProvider = nullptr;
		std::unique_ptr<DynamicUniformProvider> m_dynamicUniformProvider = nullptr;
		std::unique_ptr<RenderGraph>			m_renderGraph = nullptr;
	};
}
