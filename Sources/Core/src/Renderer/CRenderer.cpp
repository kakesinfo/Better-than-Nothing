#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CRenderer::CRenderer(CVulkanContext& vkContext)
		: m_vkContext(std::move(vkContext))
	{
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

		m_vkContext.vkSwapChain->CreateFramebuffers();

		m_vkContext.vkCommandPool = std::make_unique<CVulkanCommandPool>(&m_vkContext);

		m_vkContext.vkFrames.resize(m_vkContext.vkSettings.maxFramesInFlight);

		_createCommandBuffers();
		_createSyncObjects();

		CVulkanBufferFactory::Initialize(&m_vkContext);
		LOG_SUCCESS("Buffer factory: ok")

		m_pShaderPool = std::make_unique<CShaderPool>(m_vkContext.vkSettings.shadersFolderPath, &m_vkContext);
		LOG_SUCCESS("Shader pool: ok")

		LOG_SUCCESS("Renderer: ok");
	}

	CRenderer::~CRenderer()
	{
		CVulkanBufferFactory::Shutdown();

		m_vkContext.vkDevice->WaitIdle();
	}

	void CRenderer::RenderFrame()
	{
		m_vkContext.vkFrames[m_currentFrame].inFlightFence->Wait();

		auto device = m_vkContext.vkDevice->LogicalHandle();
		auto swapchain = m_vkContext.vkSwapChain->Handle();

		uint32 imageIndex;
		VkResult acquireResult = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
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
		_recordCommandBuffer(imageIndex);

		VkSemaphore waitSemaphores[] = { m_vkContext.vkFrames[m_currentFrame].imageAvailableSemaphore->Handle() };

		// We wait for the fragment shader to output the color before we can start writing to the image
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;

		auto commandBuffer = m_vkContext.vkFrames[m_currentFrame].commandBuffer->Handle();
		submitInfo.pCommandBuffers = &commandBuffer;

		VkSemaphore signalSemaphores[] = { m_vkContext.vkFrames[m_currentFrame].renderFinishedSemaphore->Handle() };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		auto graphicsQueue = m_vkContext.vkGraphicsQueue->Handle();
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, m_vkContext.vkFrames[m_currentFrame].inFlightFence->Handle()) != VK_SUCCESS)
			throw std::runtime_error("failed to submit draw command buffer!");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		VkResult presentResult = vkQueuePresentKHR(m_vkContext.vkPresentQueue->Handle(), &presentInfo);

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

	void CRenderer::Render(CMesh* pMesh)
	{
		m_renderQueue.push_back(pMesh);
	}

	void CRenderer::_recordCommandBuffer(uint32 imageIndex)
	{
		m_vkContext.vkFrames[m_currentFrame].commandBuffer->BeginRecording();

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_vkContext.vkRenderPass->Handle();
		renderPassInfo.framebuffer = m_vkContext.vkSwapChain->GetFramebuffer(imageIndex);
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = m_vkContext.vkSwapChain->GetExtent();

		VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		auto passContext = m_vkContext.vkFrames[m_currentFrame].commandBuffer->BeginRenderPass(&renderPassInfo);

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

		passContext->BindPipeline(m_vkContext.vkPipeline->Handle());

		for (const auto mesh : m_renderQueue)
		{
			const VkBuffer vertexBuffers[] = { mesh->GetVertexBuffer().Handle() };
			constexpr VkDeviceSize offsets[] = {0};
			passContext->BindVertexBuffers(0, 1, vertexBuffers, offsets);
			passContext->BindIndexBuffer(mesh->GetIndexBuffer().Handle(), 0, VK_INDEX_TYPE_UINT16);

			auto indices = mesh->GetIndices();
			passContext->DrawIndexed(static_cast<uint32>(indices.size()), 1, 0, 0, 0);
		}

		m_renderQueue.clear();

		m_vkContext.vkFrames[m_currentFrame].commandBuffer->EndRenderPass();
		m_vkContext.vkFrames[m_currentFrame].commandBuffer->EndRecording();
	}

	void CRenderer::_createCommandBuffers()
	{
		auto device = m_vkContext.vkDevice->LogicalHandle();

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

	void CRenderer::AddPipeline(const std::function<void(VulkanPipelineBuilder&)>& callback)
	{
		VulkanPipelineBuilder builder;
		callback(builder);

		VulkanPipelineParams params = builder.GetBuildParams();

		std::vector<CVulkanShaderModule> shaderModules;
		for (auto&[shaderPath, shaderType] : params.shadersInfos)
			shaderModules.push_back(m_pShaderPool->LoadShader(shaderPath, shaderType));

		m_vkContext.vkPipeline = std::make_unique<CVulkanPipeline>(params, shaderModules, &m_vkContext);
	}
}