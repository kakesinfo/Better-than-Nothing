#include "BetterThanNothing.hpp"
#include "Data/GlobalUniformData.hpp"

namespace BetterThanNothing
{
	CRenderer::CRenderer(VulkanContext& vkContext)
		: m_vkContext(std::move(vkContext))
	{
		CBufferManager::Initialize(&m_vkContext);

		LOG_INFO("Buffering mode: " << m_vkContext.vkSettings.maxFramesInFlight);
		LOG_INFO("VSync: " << (m_vkContext.vkSettings.enableVSync ? "enabled" : "disabled"));

		m_vkContext.vkInstance = std::make_unique<CVulkanInstance>(&m_vkContext);
		m_vkContext.vkSurface = std::make_unique<CVulkanSurface>(&m_vkContext);
		m_vkContext.vkDevice = std::make_unique<CVulkanDevice>(&m_vkContext);
		m_vkContext.vkMemoryAllocator = std::make_unique<CVulkanMemoryAllocator>(&m_vkContext);

		m_vkContext.vkSwapChain = std::make_unique<CVulkanSwapChain>(&m_vkContext);
		m_vkContext.vkSwapChain->CreateImages();
		m_vkContext.vkSwapChain->CreateImageViews();

		m_vkContext.vkRenderPass = std::make_unique<CVulkanRenderPass>(&m_vkContext);

		_createDescriptorSetLayout();

		m_vkContext.vkSwapChain->CreateFramebuffers();

		m_vkContext.vkCommandPool = std::make_unique<CVulkanCommandPool>(&m_vkContext);

		m_vkContext.vkFrames.resize(m_vkContext.vkSettings.maxFramesInFlight);

		m_pGlobalUniformProvider = std::make_unique<CGlobalUniformProvider>(&m_vkContext);
		m_pDynamicUniformProvider = std::make_unique<CDynamicUniformProvider>(&m_vkContext, 1000);

		// _createDynamicUniformBuffers();

		_createDescriptorPool();
		_createGlobalDescriptorSets();
		_createDynamicDescriptorSets();

		_createCommandBuffers();
		_createSyncObjects();

		m_pShaderPool = std::make_unique<CShaderPool>(m_vkContext.vkSettings.shadersFolderPath, &m_vkContext);
		m_pRenderCache = std::make_unique<CRenderCache>();

		LOG_SUCCESS("Renderer: ok");
	}

	CRenderer::~CRenderer()
	{
		m_vkContext.vkDevice->WaitIdle();
		CBufferManager::Shutdown();
	}

	void CRenderer::RenderFrame(CRenderList* renderList)
	{
		if (!m_vkContext.vkPipeline)
			throw std::runtime_error("No pipeline set for rendering!");

		m_vkContext.vkFrames[m_currentFrame].inFlightFence->Wait();

		const auto device = m_vkContext.vkDevice->Handle();
		const auto swapchain = m_vkContext.vkSwapChain->Handle();

		uint32 imageIndex;
		const VkResult acquireResult = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
			m_vkContext.vkFrames[m_currentFrame].imageAvailableSemaphore->Handle(), VK_NULL_HANDLE, &imageIndex);

		if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_vkContext.vkSwapChain->RecreateSwapchain();
			return;
		}

		if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("failed to acquire swap chain image!");

		m_vkContext.vkFrames[m_currentFrame].inFlightFence->Reset();
		m_vkContext.vkFrames[m_currentFrame].commandBuffer->Reset();

		_recordCommandBuffer(renderList, imageIndex);

		const VkSemaphore waitSemaphores[] = { m_vkContext.vkFrames[m_currentFrame].imageAvailableSemaphore->Handle() };

		// We wait for the fragment shader to output the color before we can start writing to the image
		constexpr VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;

		const auto commandBuffer = m_vkContext.vkFrames[m_currentFrame].commandBuffer->Handle();
		submitInfo.pCommandBuffers = &commandBuffer;

		const VkSemaphore signalSemaphores[] = { m_vkContext.vkFrames[m_currentFrame].renderFinishedSemaphore->Handle() };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		auto graphicsQueue = m_vkContext.vkGraphicsQueue->Handle();
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, m_vkContext.vkFrames[m_currentFrame].inFlightFence->Handle()) != VK_SUCCESS)
			throw std::runtime_error("failed to submit draw command buffer!");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		const VkSwapchainKHR swapChains[] = { swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		const VkResult presentResult = vkQueuePresentKHR(m_vkContext.vkPresentQueue->Handle(), &presentInfo);

		auto& pWindow = CWindowManager::GetWindow();
		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || pWindow->IsResized())
		{
			pWindow->SetResized(false);
			m_vkContext.vkSwapChain->RecreateSwapchain();
		}
		else if (presentResult != VK_SUCCESS)
			throw std::runtime_error("failed to present swap chain image!");

		m_currentFrame = (m_currentFrame + 1) % m_vkContext.vkFrames.size();
	}

	void CRenderer::_recordCommandBuffer(CRenderList* renderList, const uint32 imageIndex) const
	{
		const auto& currentFrame = m_vkContext.vkFrames[m_currentFrame];

		currentFrame.commandBuffer->BeginRecording();

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_vkContext.vkRenderPass->Handle();
		renderPassInfo.framebuffer = m_vkContext.vkSwapChain->GetFramebuffer(imageIndex);
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = m_vkContext.vkSwapChain->GetExtent();

		constexpr VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		const auto passContext = currentFrame.commandBuffer->BeginRenderPass(&renderPassInfo);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float32>(m_vkContext.vkSwapChain->GetExtent().width);
		viewport.height = static_cast<float32>(m_vkContext.vkSwapChain->GetExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		passContext->SetViewport(&viewport);

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = m_vkContext.vkSwapChain->GetExtent();
		passContext->SetScissor(&scissor);

		// Bind shader pipeline
		const VkPipelineLayout pipelineLayout = m_vkContext.vkPipeline->GetLayout();
		passContext->BindPipeline(m_vkContext.vkPipeline->Handle());

		// Bind global uniforms
		{
			const auto globalUniform = m_pGlobalUniformProvider->Get<GlobalUniformData>(m_currentFrame);
			globalUniform->view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			globalUniform->projection = glm::perspective(glm::radians(45.0f), m_vkContext.vkSwapChain->GetAspectRatio(), 0.1f, 10.0f);
			globalUniform->projection[1][1] *= -1;

			const VkDescriptorSet& globalDescriptorSet = m_vkContext.vkGlobalDescriptorSets[m_currentFrame]->Handle();
			passContext->BindDescriptorSets(pipelineLayout, 0, 1, &globalDescriptorSet);
		}

		for (uint32 i = 0; i < renderList->Size(); i++)
			{
			const auto& [pMesh, modelTransform] = renderList->Get(i);

			// Bind dynamic uniforms
			{
				const auto dynamicUniform = m_pDynamicUniformProvider->Get<DynamicUniformData>(m_currentFrame, i);
				dynamicUniform->model = modelTransform;

				const VkDescriptorSet dynamicDescriptorSet = m_vkContext.vkDynamicDescriptorSets[m_currentFrame]->Handle();
				const uint32 offset = m_pDynamicUniformProvider->GetOffset(i);
				passContext->BindDescriptorSets(pipelineLayout, 1, 1, &dynamicDescriptorSet, 1, &offset);
			}

			// Draw vertices and indices
			{
				const MeshGpuData* meshGpuData = m_pRenderCache->GetOrCreateMeshGpuData(pMesh);

				// Bind mesh vertex and index buffers
				const VkBuffer vertexBuffers[] = { meshGpuData->pVertexBuffer->Handle() };
				constexpr VkDeviceSize offsets[] = {0};
				passContext->BindVertexBuffers(0, 1, vertexBuffers, offsets);
				passContext->BindIndexBuffer(meshGpuData->pIndexBuffer->Handle(), 0, VK_INDEX_TYPE_UINT16);
			}

			// Draw call
			passContext->DrawIndexed(static_cast<uint32>(pMesh->indices.size()), 1, 0, 0, 0);
		}

		currentFrame.commandBuffer->EndRenderPass();
		currentFrame.commandBuffer->EndRecording();
	}

	void CRenderer::_createCommandBuffers()
	{
		const auto device = m_vkContext.vkDevice->Handle();

		const uint32 count = m_vkContext.vkSettings.maxFramesInFlight;

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_vkContext.vkCommandPool->Handle();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		for (uint32 i = 0; i < count; i++)
		{
			VkCommandBuffer commandBuffer;
			if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
				throw std::runtime_error("failed to allocate command buffers!");

			m_vkContext.vkFrames[i].commandBuffer = std::make_unique<CVulkanCommandBuffer>(commandBuffer);
		}

		LOG_SUCCESS("Vulkan command buffers: ok (count: " << count << ")");
	}

	void CRenderer::_createSyncObjects()
	{
		const uint32 count = m_vkContext.vkSettings.maxFramesInFlight;

		for (uint32 i = 0; i < count; i++)
		{
			auto& frame = m_vkContext.vkFrames[i];
			frame.imageAvailableSemaphore = std::make_unique<CVulkanSemaphore>(&m_vkContext);
			frame.renderFinishedSemaphore = std::make_unique<CVulkanSemaphore>(&m_vkContext);
			frame.inFlightFence = std::make_unique<CVulkanFence>(&m_vkContext);
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
		globalUniformLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding dynamicUniformLayoutBinding{};
		dynamicUniformLayoutBinding.binding = 0;
		dynamicUniformLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		dynamicUniformLayoutBinding.descriptorCount = 1;
		dynamicUniformLayoutBinding.pImmutableSamplers = nullptr;
		dynamicUniformLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		auto globalBindings = { globalUniformLayoutBinding };
		auto dynamicBindings = { dynamicUniformLayoutBinding };

		m_vkContext.vkGlobalDescriptorSetLayout = std::make_unique<CVulkanDescriptorSetLayout>(globalBindings, &m_vkContext);
		m_vkContext.vkDynamicDescriptorSetLayout = std::make_unique<CVulkanDescriptorSetLayout>(dynamicBindings, &m_vkContext);

		LOG_SUCCESS("Vulkan descriptor set layout: ok");
	}

	void CRenderer::_createDescriptorPool()
	{
		const uint32 frames = m_vkContext.vkSettings.maxFramesInFlight;

		VkDescriptorPoolSize globalPoolSize{};
		globalPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		globalPoolSize.descriptorCount = frames; // 1 global UBO par frame

		VkDescriptorPoolSize dynamicPoolSize{};
		dynamicPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		dynamicPoolSize.descriptorCount = frames; // 1 dynamic UBO par frame

		std::vector poolSizes = { globalPoolSize, dynamicPoolSize };

		m_vkContext.vkDescriptorPool = std::make_unique<CVulkanDescriptorPool>(poolSizes, &m_vkContext);

		LOG_SUCCESS("Vulkan descriptor pool: ok");
	}

	void CRenderer::_createGlobalDescriptorSets()
	{
		m_vkContext.vkGlobalDescriptorSets.resize(m_vkContext.vkSettings.maxFramesInFlight);

		for (size_t i = 0; i < m_vkContext.vkSettings.maxFramesInFlight; i++) {
			m_vkContext.vkGlobalDescriptorSets[i] = std::make_unique<CVulkanDescriptorSet>(m_vkContext.vkGlobalDescriptorSetLayout, &m_vkContext);

			VkDescriptorBufferInfo globalInfo{};
			globalInfo.buffer = m_vkContext.vkFrames[i].globalUniformBuffer->Handle();
			globalInfo.offset = 0;
			globalInfo.range = sizeof(GlobalUniformData);

			const auto& descriptorSet = m_vkContext.vkGlobalDescriptorSets[i]->Handle();

			VkWriteDescriptorSet descriptorWrite{};

			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &globalInfo;

			vkUpdateDescriptorSets(m_vkContext.vkDevice->Handle(), 1, &descriptorWrite, 0, nullptr);
		}

		LOG_SUCCESS("Vulkan global descriptor sets: ok (count: " << m_vkContext.vkGlobalDescriptorSets.size() << ")");
	}


	void CRenderer::_createDynamicDescriptorSets()
	{
		m_vkContext.vkDynamicDescriptorSets.resize(m_vkContext.vkSettings.maxFramesInFlight);

		for (size_t i = 0; i < m_vkContext.vkSettings.maxFramesInFlight; i++) {
			m_vkContext.vkDynamicDescriptorSets[i] = std::make_unique<CVulkanDescriptorSet>(m_vkContext.vkDynamicDescriptorSetLayout, &m_vkContext);

			VkDescriptorBufferInfo dynamicInfo{};
			dynamicInfo.buffer = m_vkContext.vkFrames[i].dynamicUniformBuffer->Handle();
			dynamicInfo.offset = 0;
			dynamicInfo.range = sizeof(DynamicUniformData);

			const auto& descriptorSet = m_vkContext.vkDynamicDescriptorSets[i]->Handle();

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &dynamicInfo;

			vkUpdateDescriptorSets(m_vkContext.vkDevice->Handle(), 1, &descriptorWrite, 0, nullptr);
		}

		LOG_SUCCESS("Vulkan dynamic descriptor sets: ok (count: " << m_vkContext.vkDynamicDescriptorSets.size() << ")");
	}

	void CRenderer::AddPipeline(const std::function<void(IPipelineBuilder*)>& callback)
	{
		VulkanPipelineBuilder builder;
		callback(&builder);

		VulkanPipelineParams params = builder.GetBuildParams();

		std::vector<VulkanShaderModule> shaderModules;
		for (auto&[shaderPath, shaderType] : params.shadersInfos)
			shaderModules.push_back(m_pShaderPool->LoadShader(shaderPath, shaderType));

		m_vkContext.vkPipeline = std::make_unique<CVulkanPipeline>(params, shaderModules, &m_vkContext);
	}
}
