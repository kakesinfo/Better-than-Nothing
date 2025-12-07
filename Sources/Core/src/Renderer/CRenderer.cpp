#include "BetterThanNothing.hpp"
#include "UniformBufferObjectData.hpp"

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


		_createUniformBuffers();
		_createDescriptorPool();
		_createDescriptorSets();

		_createCommandBuffers();
		_createSyncObjects();

		LOG_SUCCESS("Buffer factory: ok")

		m_pShaderPool = std::make_unique<CShaderPool>(m_vkContext.vkSettings.shadersFolderPath, &m_vkContext);
		LOG_SUCCESS("Shader pool: ok")

		m_pRenderCache = std::make_unique<CRenderCache>();

		LOG_SUCCESS("Renderer: ok");
	}

	CRenderer::~CRenderer()
	{
		m_vkContext.vkDevice->WaitIdle();
		CBufferManager::Shutdown();
	}

	void CRenderer::RenderFrame(CRenderQueue* renderQueue)
	{
		if (!m_vkContext.vkPipeline)
			throw std::runtime_error("No pipeline set for rendering!");

		m_vkContext.vkFrames[m_currentFrame].inFlightFence->Wait();

		_updateUniformBuffer();

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

		_recordCommandBuffer(renderQueue, imageIndex);

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

	void CRenderer::_recordCommandBuffer(CRenderQueue* renderQueue, const uint32 imageIndex) const
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
		passContext->BindPipeline(m_vkContext.vkPipeline->Handle());

		// Bind global uniforms
		VkDescriptorSet descriptorSet = m_vkContext.vkDescriptorSets[m_currentFrame]->Handle();
		passContext->BindDescriptorSets(
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_vkContext.vkPipeline->GetLayout(), 0, 1,
			&descriptorSet
		);

		while (!renderQueue->IsEmpty())
		{
			const auto pMesh = renderQueue->Pop().pMesh;

			const MeshGpuData* meshGpuData = m_pRenderCache->GetOrCreateMeshGpuData(pMesh);

			// Bind mesh vertex and index buffers
			const VkBuffer vertexBuffers[] = { meshGpuData->pVertexBuffer->Handle() };
			constexpr VkDeviceSize offsets[] = {0};
			passContext->BindVertexBuffers(0, 1, vertexBuffers, offsets);
			passContext->BindIndexBuffer(meshGpuData->pIndexBuffer->Handle(), 0, VK_INDEX_TYPE_UINT16);

			// Draw mesh
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

	void CRenderer::_createUniformBuffers()
	{
		const uint32 count = m_vkContext.vkSettings.maxFramesInFlight;
		for (uint32 i = 0; i < count; i++) {
			m_vkContext.vkFrames[i].uniformBuffer = CBufferManager::CreateUniformBuffer<UniformBufferObjectData>();
		}

		LOG_SUCCESS("Vulkan uniforms: ok (count: " << count << ")");
	}

	void CRenderer::_createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		auto bindings = std::vector<VkDescriptorSetLayoutBinding>{uboLayoutBinding};
		m_vkContext.vkDescriptorSetLayout = std::make_unique<CVulkanDescriptorSetLayout>(bindings, &m_vkContext);

		LOG_SUCCESS("Vulkan descriptor set layout: ok");
	}

	void CRenderer::_createDescriptorPool()
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = m_vkContext.vkSettings.maxFramesInFlight;

		m_vkContext.vkDescriptorPool = std::make_unique<CVulkanDescriptorPool>(std::vector{poolSize}, &m_vkContext);

		LOG_SUCCESS("Vulkan descriptor pool: ok");
	}

	void CRenderer::_createDescriptorSets()
	{
		m_vkContext.vkDescriptorSets.resize(m_vkContext.vkSettings.maxFramesInFlight);

		for (size_t i = 0; i < m_vkContext.vkSettings.maxFramesInFlight; i++) {
			m_vkContext.vkDescriptorSets[i] = std::make_unique<CVulkanDescriptorSet>(m_vkContext.vkDescriptorSetLayout, &m_vkContext);

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_vkContext.vkFrames[i].uniformBuffer->Handle();
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObjectData);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_vkContext.vkDescriptorSets[i]->Handle();
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(m_vkContext.vkDevice->Handle(), 1, &descriptorWrite, 0, nullptr);
		}

		LOG_SUCCESS("Vulkan descriptor sets: ok (count: " << m_vkContext.vkDescriptorSets.size() << ")");
	}

	void CRenderer::_updateUniformBuffer() const
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		const auto currentTime = std::chrono::high_resolution_clock::now();
		const float time = std::chrono::duration<float>(currentTime - startTime).count();

		const auto [width, height] = m_vkContext.vkSwapChain->GetExtent();
		const float ratio = static_cast<float>(width) / static_cast<float>(height);

		UniformBufferObjectData ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.projection = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 10.0f);
		ubo.projection[1][1] *= -1;

		m_vkContext.vkFrames[m_currentFrame].uniformBuffer->Update(ubo);
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
