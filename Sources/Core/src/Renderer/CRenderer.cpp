#include "BetterThanNothing.hpp"
#include "Data/GlobalUniformData.hpp"

namespace BetterThanNothing
{
	CRenderer::CRenderer(VulkanContext* pVkContext): m_pVkContext(pVkContext)
	{
		CBufferManager::Initialize(m_pVkContext);

		LOG_INFO("Buffering mode: " << m_pVkContext->vkSettings.maxFramesInFlight);
		LOG_INFO("VSync: " << (m_pVkContext->vkSettings.enableVSync ? "enabled" : "disabled"));

		m_pVkContext->vkInstance = std::make_unique<CVulkanInstance>(m_pVkContext);
		m_pVkContext->vkSurface = std::make_unique<CVulkanSurface>(m_pVkContext);
		m_pVkContext->vkDevice = std::make_unique<CVulkanDevice>(m_pVkContext);
		m_pVkContext->vkMemoryAllocator = std::make_unique<CVulkanMemoryAllocator>(m_pVkContext);

		LOG_INFO("Anisotropy: " << m_pVkContext->vkDevice->GetMaxAnisotropy() << "x");
		LOG_INFO("Msaa: " << static_cast<uint32>(m_pVkContext->vkDevice->GetMaxMsaaSamples()) << "x");

		m_pVkContext->vkCommandPool = std::make_unique<CVulkanCommandPool>(m_pVkContext);

		m_pVkContext->vkSwapChain = std::make_unique<CVulkanSwapChain>(m_pVkContext);
		m_pVkContext->vkSwapChain->CreateImages();
		m_pVkContext->vkSwapChain->CreateImageViews();
		m_pVkContext->vkSwapChain->CreateColorResources();
		m_pVkContext->vkSwapChain->CreateDepthResources();

		m_pVkContext->vkRenderPass = std::make_unique<CVulkanRenderPass>(m_pVkContext);

		m_pVkContext->vkSwapChain->CreateFramebuffers();

		m_pVkContext->vkFrames.resize(m_pVkContext->vkSettings.maxFramesInFlight);

		m_pGlobalUniformProvider = std::make_unique<CGlobalUniformProvider>(m_pVkContext);
		m_pDynamicUniformProvider = std::make_unique<CDynamicUniformProvider>(m_pVkContext, 100000);

		_createDescriptorSetLayout();
		_createDescriptorPool();
		_createGlobalDescriptorSets();
		_createDynamicDescriptorSets();

		_createCommandBuffers();
		_createSyncObjects();

		LOG_SUCCESS("Renderer: ok");
	}

	CRenderer::~CRenderer()
	{
		m_pVkContext->vkDevice->WaitIdle();
		CBufferManager::Shutdown();
	}

	void CRenderer::RenderFrame(CRenderList* renderList, CCamera* camera)
	{
		if (!m_pVkContext->vkPipeline)
			throw std::runtime_error("No pipeline set for rendering!");

		m_pVkContext->vkFrames[m_currentFrame].inFlightFence->Wait();

		const auto device = m_pVkContext->vkDevice->Handle();
		const auto swapchain = m_pVkContext->vkSwapChain->Handle();

		uint32 imageIndex;
		const VkResult acquireResult = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
			m_pVkContext->vkFrames[m_currentFrame].imageAvailableSemaphore->Handle(), VK_NULL_HANDLE, &imageIndex);

		if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_pVkContext->vkSwapChain->RecreateSwapchain();
			return;
		}

		if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("failed to acquire swap chain image!");

		m_pVkContext->vkFrames[m_currentFrame].inFlightFence->Reset();
		m_pVkContext->vkFrames[m_currentFrame].commandBuffer->Reset();

		_recordCommandBuffer(renderList, camera, imageIndex);

		const VkSemaphore waitSemaphores[] = { m_pVkContext->vkFrames[m_currentFrame].imageAvailableSemaphore->Handle() };

		// We wait for the fragment shader to output the color before we can start writing to the image
		constexpr VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;

		const auto commandBuffer = m_pVkContext->vkFrames[m_currentFrame].commandBuffer->Handle();
		submitInfo.pCommandBuffers = &commandBuffer;

		const VkSemaphore signalSemaphores[] = { m_pVkContext->vkFrames[m_currentFrame].renderFinishedSemaphore->Handle() };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		auto graphicsQueue = m_pVkContext->vkGraphicsQueue->Handle();
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, m_pVkContext->vkFrames[m_currentFrame].inFlightFence->Handle()) != VK_SUCCESS)
			throw std::runtime_error("failed to submit draw command buffer!");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		const VkSwapchainKHR swapChains[] = { swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		const VkResult presentResult = vkQueuePresentKHR(m_pVkContext->vkPresentQueue->Handle(), &presentInfo);

		auto& pWindow = CWindowManager::GetWindow();
		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || pWindow->IsResized())
		{
			pWindow->SetResized(false);
			m_pVkContext->vkSwapChain->RecreateSwapchain();
		}
		else if (presentResult != VK_SUCCESS)
			throw std::runtime_error("failed to present swap chain image!");

		m_currentFrame = (m_currentFrame + 1) % m_pVkContext->vkFrames.size();
	}

	void CRenderer::_recordCommandBuffer(CRenderList* renderList, CCamera* camera, const uint32 imageIndex) const
	{
		const auto& currentFrame = m_pVkContext->vkFrames[m_currentFrame];

		currentFrame.commandBuffer->BeginRecording();

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_pVkContext->vkRenderPass->Handle();
		renderPassInfo.framebuffer = m_pVkContext->vkSwapChain->GetFramebuffer(imageIndex);
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = m_pVkContext->vkSwapChain->GetExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
		clearValues[1].depthStencil = {1.0f, 0};

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		const auto passContext = currentFrame.commandBuffer->BeginRenderPass(&renderPassInfo);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float32>(m_pVkContext->vkSwapChain->GetExtent().width);
		viewport.height = static_cast<float32>(m_pVkContext->vkSwapChain->GetExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		passContext->SetViewport(&viewport);

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = m_pVkContext->vkSwapChain->GetExtent();
		passContext->SetScissor(&scissor);

		// Bind shader pipeline
		VkPipelineLayout pipelineLayout = m_pVkContext->vkPipeline->GetLayout();
		passContext->BindPipeline(m_pVkContext->vkPipeline->Handle());

		// Bind global uniforms
		{
			const auto globalUniform = m_pGlobalUniformProvider->Get<GlobalUniformData>(m_currentFrame);
			globalUniform->view = camera->GetViewMatrix();
			globalUniform->projection = camera->GetProjectionMatrix(m_pVkContext->vkSwapChain->GetAspectRatio());
			globalUniform->lightPosition = glm::vec3(0.0f, 10.0f, 0.0f);
			globalUniform->cameraPosition = camera->GetPosition();

			const VkDescriptorSet& globalDescriptorSet = m_pVkContext->vkGlobalDescriptorSets[m_currentFrame]->Handle();
			passContext->BindDescriptorSets(pipelineLayout, 0, 1, &globalDescriptorSet);
		}

		for (uint32 i = 0; i < renderList->Size(); i++)
		{
			const auto& [pMeshGpuData, pTextureGpuData, transform] = renderList->Get(i);

			// Bind dynamic uniforms
			{
				const auto dynamicUniform = m_pDynamicUniformProvider->Get<DynamicUniformData>(m_currentFrame, i);
				dynamicUniform->model = transform;

				VkDescriptorSet dynamicDescriptorSet = m_pVkContext->vkDynamicDescriptorSets[m_currentFrame]->Handle();
				const uint32 offset = m_pDynamicUniformProvider->GetOffset(i);
				passContext->BindDescriptorSets(pipelineLayout, 1, 1, &dynamicDescriptorSet, 1, &offset);
			}

			// Bind texture sampler
			{
				const VkDescriptorSet& samplerDescriptorSet = pTextureGpuData->pVkDescriptorSet->Handle();
				passContext->BindDescriptorSets(pipelineLayout, 2, 1, &samplerDescriptorSet);
				// TODO: bind only if different from last frame
			}

			// Draw vertices and indices
			{
				// Bind mesh vertex and index buffers
				const VkBuffer vertexBuffers[] = { pMeshGpuData->pVertexBuffer->Handle() };
				constexpr VkDeviceSize offsets[] = {0};
				passContext->BindVertexBuffers(0, 1, vertexBuffers, offsets);
				passContext->BindIndexBuffer(pMeshGpuData->pIndexBuffer->Handle(), 0, VK_INDEX_TYPE_UINT32);
			}

			// Draw call
			passContext->DrawIndexed(pMeshGpuData->indexCount, 1, 0, 0, 0);
		}

		currentFrame.commandBuffer->EndRenderPass();
		currentFrame.commandBuffer->EndRecording();
	}

	void CRenderer::SetRenderCache(CRenderCache* pRenderCache)
	{
		m_pRenderCache = pRenderCache;
	}

	void CRenderer::_createCommandBuffers() const
	{
		const auto device = m_pVkContext->vkDevice->Handle();
		const uint32 count = m_pVkContext->vkSettings.maxFramesInFlight;

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_pVkContext->vkCommandPool->Handle();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		for (uint32 i = 0; i < count; i++)
		{
			VkCommandBuffer commandBuffer;
			if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
				throw std::runtime_error("failed to allocate command buffers!");

			m_pVkContext->vkFrames[i].commandBuffer = std::make_unique<CVulkanCommandBuffer>(commandBuffer);
		}

		LOG_SUCCESS("Vulkan command buffers: ok (count: " << count << ")");
	}

	void CRenderer::_createSyncObjects()
	{
		const uint32 count = m_pVkContext->vkSettings.maxFramesInFlight;

		for (uint32 i = 0; i < count; i++)
		{
			auto& frame = m_pVkContext->vkFrames[i];
			frame.imageAvailableSemaphore = std::make_unique<CVulkanSemaphore>(m_pVkContext);
			frame.renderFinishedSemaphore = std::make_unique<CVulkanSemaphore>(m_pVkContext);
			frame.inFlightFence = std::make_unique<CVulkanFence>(m_pVkContext);
		}

		LOG_SUCCESS("Vulkan sync objects: ok (count: " << count << ")");
	}

	void CRenderer::_createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding globalUniformLayoutBinding{};
		globalUniformLayoutBinding.binding = 0;
		globalUniformLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		globalUniformLayoutBinding.descriptorCount = 1;
		globalUniformLayoutBinding.pImmutableSamplers = nullptr;
		globalUniformLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding dynamicUniformLayoutBinding{};
		dynamicUniformLayoutBinding.binding = 0;
		dynamicUniformLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		dynamicUniformLayoutBinding.descriptorCount = 1;
		dynamicUniformLayoutBinding.pImmutableSamplers = nullptr;
		dynamicUniformLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 0;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		auto globalBindings = { globalUniformLayoutBinding };
		auto dynamicBindings = { dynamicUniformLayoutBinding };
		auto samplerBindings = { samplerLayoutBinding };

		m_pVkContext->vkGlobalDescriptorSetLayout = std::make_unique<CVulkanDescriptorSetLayout>(globalBindings, m_pVkContext);
		m_pVkContext->vkDynamicDescriptorSetLayout = std::make_unique<CVulkanDescriptorSetLayout>(dynamicBindings, m_pVkContext);
		m_pVkContext->vkSamplerDescriptorSetLayout = std::make_unique<CVulkanDescriptorSetLayout>(samplerBindings, m_pVkContext);

		LOG_SUCCESS("Vulkan descriptor set layout: ok");
	}

	void CRenderer::_createDescriptorPool()
	{
		const uint32 frames = m_pVkContext->vkSettings.maxFramesInFlight;

		VkDescriptorPoolSize globalPoolSize{};
		globalPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		globalPoolSize.descriptorCount = frames; // 1 global UBO par frame

		VkDescriptorPoolSize dynamicPoolSize{};
		dynamicPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		dynamicPoolSize.descriptorCount = frames; // 1 dynamic UBO par frame

		VkDescriptorPoolSize samplerPoolSize{};
		samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerPoolSize.descriptorCount = 1000; // TODO: adjust based on expected number of textures

		std::vector poolSizes = { globalPoolSize, dynamicPoolSize, samplerPoolSize };

		m_pVkContext->vkDescriptorPool = std::make_unique<CVulkanDescriptorPool>(poolSizes, m_pVkContext);

		LOG_SUCCESS("Vulkan descriptor pool: ok");
	}

	void CRenderer::_createGlobalDescriptorSets()
	{
		m_pVkContext->vkGlobalDescriptorSets.resize(m_pVkContext->vkSettings.maxFramesInFlight);

		for (size_t i = 0; i < m_pVkContext->vkSettings.maxFramesInFlight; i++) {
			m_pVkContext->vkGlobalDescriptorSets[i] = std::make_unique<CVulkanDescriptorSet>(m_pVkContext->vkGlobalDescriptorSetLayout, m_pVkContext);

			VkDescriptorBufferInfo globalInfo{};
			globalInfo.buffer = m_pVkContext->vkFrames[i].globalUniformBuffer->Handle();
			globalInfo.offset = 0;
			globalInfo.range = sizeof(GlobalUniformData);

			const auto& descriptorSet = m_pVkContext->vkGlobalDescriptorSets[i]->Handle();

			VkWriteDescriptorSet descriptorWrite{};

			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &globalInfo;

			vkUpdateDescriptorSets(m_pVkContext->vkDevice->Handle(), 1, &descriptorWrite, 0, nullptr);
		}

		LOG_SUCCESS("Vulkan global descriptor sets: ok (count: " << m_pVkContext->vkGlobalDescriptorSets.size() << ")");
	}


	void CRenderer::_createDynamicDescriptorSets()
	{
		m_pVkContext->vkDynamicDescriptorSets.resize(m_pVkContext->vkSettings.maxFramesInFlight);

		for (size_t i = 0; i < m_pVkContext->vkSettings.maxFramesInFlight; i++) {
			m_pVkContext->vkDynamicDescriptorSets[i] = std::make_unique<CVulkanDescriptorSet>(m_pVkContext->vkDynamicDescriptorSetLayout, m_pVkContext);

			VkDescriptorBufferInfo dynamicInfo{};
			dynamicInfo.buffer = m_pVkContext->vkFrames[i].dynamicUniformBuffer->Handle();
			dynamicInfo.offset = 0;
			dynamicInfo.range = sizeof(DynamicUniformData);

			const auto& descriptorSet = m_pVkContext->vkDynamicDescriptorSets[i]->Handle();

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &dynamicInfo;

			vkUpdateDescriptorSets(m_pVkContext->vkDevice->Handle(), 1, &descriptorWrite, 0, nullptr);
		}

		LOG_SUCCESS("Vulkan dynamic descriptor sets: ok (count: " << m_pVkContext->vkDynamicDescriptorSets.size() << ")");
	}

	void CRenderer::AddPipeline(const std::function<void(IPipelineBuilder*)>& callback)
	{
		VulkanPipelineBuilder builder;
		callback(&builder);

		VulkanPipelineParams params = builder.GetBuildParams();

		std::vector<const ShaderGpuData*> shaderModules;
		shaderModules.reserve(params.shadersInfos.size());

		for (auto&[shaderPath, shaderType] : params.shadersInfos) {
			const std::string fullShaderPath = m_pVkContext->vkSettings.shadersFolderPath + shaderPath;
			const ShaderGpuData* shaderGpuData = m_pRenderCache->GetOrCreateShaderGpuData({shaderType, fullShaderPath});
			shaderModules.push_back(shaderGpuData);
		}

		m_pVkContext->vkPipeline = std::make_unique<CVulkanPipeline>(params, shaderModules, m_pVkContext);
	}
}
