/*
 *	Better than Nothing
 *	Copyright (C) 2026 kakesinfo
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

#include "RenderingDevice/RenderingDevice.hpp"
#include "RenderingApi/Resources/Buffer.hpp"
#include "RenderingApi/Resources/DescriptorSet.hpp"
#include "RenderingApi/Resources/Sampler.hpp"
#include "RenderingApi/Resources/Texture.hpp"
#include "RenderingDevice_Internals.hpp"
#include "StagingBuffer.hpp"
#include <Module_Base/Logger.hpp>
#include <Module_Platform/Manager/WindowManager.hpp>
#include <Module_Platform/Window.hpp>
#include <RenderingApi.hpp>
#include <cmath>

namespace BetterThanNothing
{
	RenderingDevice::RenderingDevice(const RenderingDeviceCreateInfo& createInfo) : m_createInfo(createInfo)
	{
		const auto& window = WindowManager::GetWindow();
		const auto [windowWidth, windowHeight] = WindowManager::GetWindow()->GetFramebufferSize();

		m_self = std::make_unique<RenderingDeviceInternals>();

		m_self->instance = RenderingApi::Instance::Create({.enableValidation = true});
		m_self->surface = RenderingApi::Surface::Create({window->Handle(), m_self->instance.get()});
		m_self->device = RenderingApi::Device::Create({m_self->instance.get(), m_self->surface.get()});
		m_self->graphicsQueue = m_self->device->CreateGraphicsQueue();
		m_self->presentQueue = m_self->device->CreatePresentQueue();
		m_self->allocator = RenderingApi::Allocator::Create({m_self->instance.get(), m_self->device.get()});

		m_self->commandPool = RenderingApi::CommandPool::Create({m_self->device.get(), m_self->graphicsQueue.get()});

		m_self->swapChain = RenderingApi::SwapChain::Create({
			m_self->surface.get(),
			m_self->device.get(),
			m_self->allocator.get(),
			windowWidth,
			windowHeight,
			createInfo.bufferingMode,
		});

		m_maxFramesInFlight = m_self->swapChain->GetImageCount();

		Logger::Log<INFO>("Vendor: {}", m_self->device->GetVendorName());
		Logger::Log<INFO>("Device: {}", m_self->device->GetDeviceName());
		Logger::Log<INFO>("API Version: {}", m_self->device->GetApiVersion());
		Logger::Log<INFO>("Buffering mode: {}", static_cast<uint8_t>(createInfo.bufferingMode));
		Logger::Log<INFO>("Max frame in-flight: {}", m_maxFramesInFlight);
		Logger::Log<INFO>("VSync: {}", createInfo.bufferingMode != EBufferingMode::Immediate ? "enabled" : "disabled");
		Logger::Log<INFO>("Anisotropy: {}x", m_self->device->GetMaxAnisotropy());
		Logger::Log<INFO>("Msaa: {}x", static_cast<uint32_t>(m_self->device->GetMaxMsaaSamples()));

		m_self->descriptorPool = RenderingApi::DescriptorPool::Builder(m_self->device.get())
									 .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_maxFramesInFlight)
									 .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
									 .AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, m_maxFramesInFlight)
									 .Build();

		Internal_RegisterDescriptorLayout(DescriptorSetLayoutType::GlobalUniforms, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		Internal_RegisterDescriptorLayout(DescriptorSetLayoutType::Textures, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		Internal_RegisterDescriptorLayout(DescriptorSetLayoutType::PointLights, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);

		m_self->stagingBuffer = StagingBuffer::Create(m_self->allocator.get(), 128);

		Internal_CreateCommandBuffers();
		Internal_CreateSyncObjects();
	}

	RenderingDevice::~RenderingDevice()
	{
		m_self->device->WaitIdle();

		m_self->samplersLUT.clear();

		for (std::uint32_t i = 0; i < m_self->samplers.Size(); i++)
		{
			m_self->device->DestroySampler(m_self->samplers.Get(i)->value);
			m_self->samplers.Destroy(i);
		}

		for (std::uint32_t i = 0; i < m_self->buffers.Size(); i++)
		{
			const auto buffer = m_self->buffers.Get(i);
			m_self->allocator->DestroyBuffer(buffer->value, buffer->allocation);
			m_self->buffers.Destroy(i);
		}

		for (std::uint32_t i = 0; i < m_self->textures.Size(); i++)
		{
			const auto texture = m_self->textures.Get(i);
			m_self->device->DestroyImageView(texture->defaultView);
			m_self->allocator->DestroyImage(texture->value, texture->allocation);
			m_self->textures.Destroy(i);
		}
	};

	void RenderingDevice::Internal_RegisterDescriptorLayout(const DescriptorSetLayoutType key, const VkDescriptorType type, const VkShaderStageFlags stageFlags)
	{
		m_self->descriptorLayouts[key] = RenderingApi::DescriptorSetLayout::Create({m_self->device.get(), type, 1, stageFlags});
	}

	std::unique_ptr<RenderingDevice> RenderingDevice::Create(const RenderingDeviceCreateInfo& createInfo)
	{
		return std::make_unique<RenderingDevice>(createInfo);
	}

	void RenderingDevice::Internal_CreateCommandBuffers()
	{
		m_self->commandBuffers.resize(m_maxFramesInFlight);
		for (uint32_t i = 0; i < m_maxFramesInFlight; i++)
			m_self->commandBuffers[i] = RenderingApi::CommandBuffer::Create({m_self->device.get(), m_self->commandPool.get()});

		Logger::Log<SUCCESS>("Vulkan command buffers: ok");
	}

	void RenderingDevice::Internal_CreateSyncObjects()
	{
		m_self->imageAvailableSemaphores.resize(m_maxFramesInFlight);
		m_self->inFlightFences.resize(m_maxFramesInFlight);

		for (uint32_t i = 0; i < m_maxFramesInFlight; i++)
		{
			m_self->imageAvailableSemaphores[i] = RenderingApi::Semaphore::Create({m_self->device.get()});
			m_self->inFlightFences[i] = RenderingApi::Fence::Create({m_self->device.get(), true});
		}

		Logger::Log<SUCCESS>("Vulkan sync objects: ok");
	}

	std::optional<uint32_t> RenderingDevice::BeginFrame()
	{
		m_self->inFlightFences.at(m_currentFrameIndex)->Wait();
		FlushDescriptorSetUpdateRequests();

		const auto acquireResult = m_self->swapChain->AcquireNextImage(m_self->imageAvailableSemaphores.at(m_currentFrameIndex)->Handle());

		if (!acquireResult.has_value())
		{
			const auto [windowWidth, windowHeight] = WindowManager::GetWindow()->GetFramebufferSize();
			m_self->device->WaitIdle();
			m_self->swapChain->RecreateSwapChain(windowWidth, windowHeight);
			return std::nullopt;
		}

		const uint32_t imageIndex = acquireResult.value();

		m_self->inFlightFences.at(m_currentFrameIndex)->Reset();
		m_self->commandBuffers.at(m_currentFrameIndex)->Reset();
		m_self->commandBuffers.at(m_currentFrameIndex)->BeginRecording();

		return imageIndex;
	}

	void RenderingDevice::EndFrame(const uint32_t imageIndex)
	{
		m_self->commandBuffers.at(m_currentFrameIndex)->EndRecording();

		const auto					   commandBuffer = m_self->commandBuffers.at(m_currentFrameIndex)->Handle();
		const VkSemaphore			   waitSemaphores[] = {m_self->imageAvailableSemaphores.at(m_currentFrameIndex)->Handle()};
		constexpr VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		const VkSemaphore			   signalSemaphores[] = {m_self->swapChain->GetImageFinishedSemaphore(imageIndex)};

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		auto graphicsQueue = m_self->graphicsQueue->Handle();
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, m_self->inFlightFences.at(m_currentFrameIndex)->Handle()) != VK_SUCCESS)
			throw std::runtime_error("failed to submit draw command buffer!");

		const VkSwapchainKHR swapChains[] = {m_self->swapChain->Handle()};

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		const VkResult presentResult = vkQueuePresentKHR(m_self->presentQueue->Handle(), &presentInfo);

		auto& window = WindowManager::GetWindow();
		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || window->IsResized())
		{
			const auto [windowWidth, windowHeight] = window->GetFramebufferSize();
			window->SetResized(false);
			m_self->device->WaitIdle();
			m_self->swapChain->RecreateSwapChain(windowWidth, windowHeight);
		}
		else if (presentResult != VK_SUCCESS)
			throw std::runtime_error("failed to present swap chain image!");

		m_currentFrameIndex = (m_currentFrameIndex + 1) % m_maxFramesInFlight;
	}

	void RenderingDevice::WaitIdle() const
	{
		m_self->device->WaitIdle();
	}

	[[nodiscard]] uint32_t RenderingDevice::GetCurrentFrameIndex() const
	{
		return m_currentFrameIndex;
	}

	RenderingApi::CommandBuffer* RenderingDevice::GetCurrentCommandBuffer() const
	{
		return m_self->commandBuffers[m_currentFrameIndex].get();
	}

	RenderingApi::Instance* RenderingDevice::GetInstance() const
	{
		return m_self->instance.get();
	}

	RenderingApi::Device* RenderingDevice::GetDevice() const
	{
		return m_self->device.get();
	}

	RenderingApi::Queue* RenderingDevice::GetGraphicsQueue() const
	{
		return m_self->graphicsQueue.get();
	}

	RenderingApi::Allocator* RenderingDevice::GetAllocator() const
	{
		return m_self->allocator.get();
	}

	RenderingApi::SwapChain* RenderingDevice::GetSwapChain() const
	{
		return m_self->swapChain.get();
	}

	auto RenderingDevice::GetMaxFramesInFlight() const -> uint32_t
	{
		return m_maxFramesInFlight;
	}

	auto RenderingDevice::GetAspectRatio() const -> float
	{
		return m_self->swapChain->GetAspectRatio();
	}

	RenderingApi::DescriptorSetLayout* RenderingDevice::GetDescriptorSetLayout(DescriptorSetLayoutType key) const
	{
		if (!m_self->descriptorLayouts.contains(key))
			throw std::runtime_error("Cannot find descriptorSetLayout");
		return m_self->descriptorLayouts.at(key).get();
	}
}
