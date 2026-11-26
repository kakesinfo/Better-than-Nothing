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
#include "Platform/Platform.hpp"
#include "RenderingApi/Resources/Buffer.hpp"
#include "RenderingApi/Resources/DescriptorSet.hpp"
#include "RenderingApi/Resources/Pipeline.hpp"
#include "RenderingApi/Resources/Sampler.hpp"
#include "RenderingApi/Resources/Texture.hpp"
#include "RenderingDevice/Blueprint/PipelineLayoutBlueprint.hpp"
#include "RenderingDevice/DrawStream.hpp"
#include "RenderingDevice_Internals.hpp"
#include "StagingBuffer.hpp"

#include <Common/Logger.hpp>
#include <RenderingApi.hpp>
#include <array>
#include <cmath>

namespace BetterThanNothing
{
	static VkDescriptorType ToVkDescriptorType(const DescriptorType type)
	{
		switch (type)
		{
			case DescriptorType::UniformBuffer:
				return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			case DescriptorType::StorageBuffer:
				return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			case DescriptorType::CombinedImageSampler:
				return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			case DescriptorType::StorageImage:
				return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			case DescriptorType::AccelerationStructure:
				return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		}
		throw std::runtime_error("Unknown DescriptorType");
	}

	static VkShaderStageFlags ToVkShaderStageFlags(const ShaderStageFlags stages)
	{
		VkShaderStageFlags flags = 0;
		if (stages & ShaderStageFlags::Vertex)
			flags |= VK_SHADER_STAGE_VERTEX_BIT;
		if (stages & ShaderStageFlags::Geometry)
			flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
		if (stages & ShaderStageFlags::Fragment)
			flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
		if (stages & ShaderStageFlags::Compute)
			flags |= VK_SHADER_STAGE_COMPUTE_BIT;
		return flags;
	}

	static VkAttachmentLoadOp ToVkLoadOp(const LoadOp op)
	{
		switch (op)
		{
			case LoadOp::Load:
				return VK_ATTACHMENT_LOAD_OP_LOAD;
			case LoadOp::Clear:
				return VK_ATTACHMENT_LOAD_OP_CLEAR;
			case LoadOp::DontCare:
				return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		}
		return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}

	static VkAttachmentStoreOp ToVkStoreOp(const StoreOp op)
	{
		switch (op)
		{
			case StoreOp::Store:
				return VK_ATTACHMENT_STORE_OP_STORE;
			case StoreOp::DontCare:
				return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}
		return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}

	static VkImageLayout ToVkImageLayout(const ImageLayout layout)
	{
		switch (layout)
		{
			case ImageLayout::Undefined:
				return VK_IMAGE_LAYOUT_UNDEFINED;
			case ImageLayout::ColorAttachment:
				return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			case ImageLayout::DepthStencilAttachment:
				return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			case ImageLayout::ShaderReadOnly:
				return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			case ImageLayout::TransferSrc:
				return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			case ImageLayout::TransferDst:
				return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			case ImageLayout::General:
				return VK_IMAGE_LAYOUT_GENERAL;
			case ImageLayout::Present:
				return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}
		return VK_IMAGE_LAYOUT_UNDEFINED;
	}

	RenderingDevice::RenderingDevice(const RenderingDeviceCreateInfo& createInfo) : m_createInfo(createInfo)
	{
		const auto [windowWidth, windowHeight] = m_createInfo.platform->GetWindowSize();

		m_self = std::make_unique<RenderingDeviceInternals>();

		m_self->instance = RenderingApi::Instance::Create({.enableValidation = true});
		m_self->surface = RenderingApi::Surface::Create({m_createInfo.platform->GetWindow(), m_self->instance.get()});
		m_self->device = RenderingApi::Device::Create({m_self->instance.get(), m_self->surface.get()});
		m_self->graphicsQueue = m_self->device->CreateGraphicsQueue();
		m_self->presentQueue = m_self->device->CreatePresentQueue();
		m_self->allocator = RenderingApi::Allocator::Create({m_self->instance.get(), m_self->device.get()});

		m_self->commandPool = RenderingApi::CommandPool::Create({m_self->device.get(), m_self->graphicsQueue.get()});

		m_self->swapChain = RenderingApi::SwapChain::Create({
			m_self->surface.get(),
			m_self->device.get(),
			static_cast<uint32_t>(windowWidth),
			static_cast<uint32_t>(windowHeight),
			createInfo.bufferingMode,
		});

		m_maxFramesInFlight = m_self->swapChain->GetImageCount();
		m_self->frameDeletionQueues.resize(m_maxFramesInFlight);
		m_self->swapChainImageLayouts.assign(m_maxFramesInFlight, VK_IMAGE_LAYOUT_UNDEFINED);

		Logger::Log<INFO>("Vendor: {}", m_self->device->GetVendorName());
		Logger::Log<INFO>("Device: {}", m_self->device->GetDeviceName());
		Logger::Log<INFO>("API Version: {}", m_self->device->GetApiVersion());
		Logger::Log<INFO>("Buffering mode: {}", static_cast<uint8_t>(createInfo.bufferingMode));
		Logger::Log<INFO>("Max frame in-flight: {}", m_maxFramesInFlight);
		Logger::Log<INFO>("VSync: {}", createInfo.bufferingMode != EBufferingMode::Immediate ? "enabled" : "disabled");
		Logger::Log<INFO>("Anisotropy: {}x", m_self->device->GetMaxAnisotropy());
		Logger::Log<INFO>("Msaa: {}x", static_cast<uint32_t>(m_self->device->GetMaxMsaaSamples()));

		m_self->descriptorAllocator = std::make_unique<DescriptorAllocator>(m_self->device.get(), 512);
		m_self->stagingBuffer = StagingBuffer::Create(m_self->allocator.get(), 128);
		m_self->uploadCommandBuffer = RenderingApi::CommandBuffer::Create({m_self->device.get(), m_self->commandPool.get()});

		Internal_CreateCommandBuffers();
		Internal_CreateSyncObjects();
	}

	RenderingDevice::~RenderingDevice()
	{
		m_self->shuttingDown = true;

		if (m_self->uploadRecording)
			Internal_FlushUploadBuffer();

		m_self->device->WaitIdle();

		// Drain all deferred frame deletion queues first
		for (auto& queue : m_self->frameDeletionQueues)
			queue.Flush();

		// Destroy any remaining active resources
		m_self->pipelines.ForEachActive([&](const uint32_t idx, RenderingApi::Pipeline& pipeline) {
			if (pipeline.pipeline != VK_NULL_HANDLE)
				vkDestroyPipeline(m_self->device->Handle(), pipeline.pipeline, nullptr);
			m_self->pipelines.Destroy(idx);
		});

		m_self->shaders.ForEachActive([&](const uint32_t idx, RenderingApi::Shader& shader) {
			m_self->device->DestroyShaderModule(shader.module);
			m_self->shaders.Destroy(idx);
		});

		m_self->pipelineLayouts.clear();

		m_self->samplersLUT.clear();
		m_self->samplers.ForEachActive([&](const uint32_t idx, RenderingApi::Sampler& sampler) {
			m_self->device->DestroySampler(sampler.value);
			m_self->samplers.Destroy(idx);
		});

		m_self->buffers.ForEachActive([&](const uint32_t idx, RenderingApi::Buffer& buffer) {
			m_self->allocator->DestroyBuffer(buffer.value, buffer.allocation);
			m_self->buffers.Destroy(idx);
		});

		m_self->textures.ForEachActive([&](const uint32_t idx, RenderingApi::Texture& texture) {
			m_self->device->DestroyImageView(texture.defaultView);
			m_self->allocator->DestroyImage(texture.value, texture.allocation);
			m_self->textures.Destroy(idx);
		});

		m_self->accelerationStructures.ForEachActive([&](const uint32_t idx, RenderingApi::AccelerationStructure& as) {
			m_self->device->DestroyAccelerationStructure(as.handle);
			m_self->allocator->DestroyBuffer(as.buffer, as.allocation);
			m_self->accelerationStructures.Destroy(idx);
		});
	}

	std::unique_ptr<RenderingDevice> RenderingDevice::Create(const RenderingDeviceCreateInfo& createInfo)
	{
		return std::make_unique<RenderingDevice>(createInfo);
	}

	void RenderingDevice::Internal_EnqueueDeletion(std::function<void()>&& deletor)
	{
		if (m_self->shuttingDown || m_self->frameDeletionQueues.empty())
		{
			deletor();
			return;
		}

		m_self->frameDeletionQueues[m_currentFrameIndex].Push(std::move(deletor));
	}

	void RenderingDevice::BeginUploadBatch()
	{
		++m_self->uploadBatchDepth;
	}

	void RenderingDevice::EndUploadBatch()
	{
		if (m_self->uploadBatchDepth == 0)
			return;

		--m_self->uploadBatchDepth;
		if (m_self->uploadBatchDepth == 0 && m_self->uploadRecording)
			Internal_FlushUploadBuffer();
	}

	void RenderingDevice::Internal_EnsureUploadBuffer(const uint64_t requiredStagingBytes, const uint64_t alignment)
	{
		if (m_self->uploadRecording && !m_self->stagingBuffer->CanFit(requiredStagingBytes, alignment))
			Internal_FlushUploadBuffer();

		if (!m_self->uploadRecording)
		{
			m_self->uploadCommandBuffer->Reset();
			m_self->uploadCommandBuffer->BeginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			m_self->uploadRecording = true;
		}
	}

	void RenderingDevice::Internal_FlushUploadBuffer()
	{
		if (!m_self->uploadRecording)
			return;

		m_self->uploadCommandBuffer->EndRecording();
		m_self->uploadRecording = false;

		const VkCommandBuffer vkCmd = m_self->uploadCommandBuffer->Handle();

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &vkCmd;

		const auto fence = RenderingApi::Fence::Create({m_self->device.get(), false});
		if (vkQueueSubmit(m_self->graphicsQueue->Handle(), 1, &submitInfo, fence->Handle()) != VK_SUCCESS)
			throw std::runtime_error("Failed to submit batched upload command buffer!");

		fence->Wait();
		m_self->stagingBuffer->Reset();
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

	void RenderingDevice::CreateDescriptorSetLayout(const std::string& name, const DescriptorType type, const ShaderStageFlags stageFlags)
	{
		const DescriptorBindingDesc binding{.binding = 0, .type = type, .count = 1, .stages = stageFlags};
		CreateDescriptorSetLayout(name, std::span<const DescriptorBindingDesc>(&binding, 1));
	}

	void RenderingDevice::CreateDescriptorSetLayout(const std::string& name, const std::span<const DescriptorBindingDesc> bindings)
	{
		if (m_self->descriptorLayouts.contains(name))
			throw std::runtime_error("Descriptor layout already exists: " + name);

		RenderingApi::DescriptorSetLayoutCreateInfo info{};
		info.device = m_self->device.get();
		info.bindings.reserve(bindings.size());
		for (const auto& b : bindings)
		{
			VkDescriptorSetLayoutBinding vkBinding{};
			vkBinding.binding = b.binding;
			vkBinding.descriptorType = ToVkDescriptorType(b.type);
			vkBinding.descriptorCount = b.count;
			vkBinding.stageFlags = ToVkShaderStageFlags(b.stages);
			info.bindings.push_back(vkBinding);
		}

		m_self->descriptorLayouts[name] = RenderingApi::DescriptorSetLayout::Create(info);
	}

	[[nodiscard]] RenderingApi::DescriptorSetLayout* RenderingDevice::GetDescriptorSetLayout(const std::string& name) const
	{
		const auto it = m_self->descriptorLayouts.find(name);
		if (it == m_self->descriptorLayouts.end())
			throw std::runtime_error("Descriptor layout does not exist: " + name);

		return it->second.get();
	}

	void RenderingDevice::CreatePipelineLayout(const std::string& name, const std::function<void(PipelineLayoutBlueprint*)>& callback)
	{
		PipelineLayoutBlueprint blueprint(this);
		callback(&blueprint);

		m_self->pipelineLayouts[name] = blueprint.Build();
	}

	RenderingApi::PipelineLayout* RenderingDevice::GetPipelineLayout(const std::string& name) const
	{
		const auto it = m_self->pipelineLayouts.find(name);
		if (it == m_self->pipelineLayouts.end())
			throw std::runtime_error("Pipeline layout does not exist: " + name);

		return it->second.get();
	}

	std::optional<uint32_t> RenderingDevice::BeginFrame()
	{
		const auto [windowWidth, windowHeight] = m_createInfo.platform->GetWindowSize();

		if (windowWidth <= 0 || windowHeight <= 0)
			return std::nullopt;

		if (m_self->uploadRecording)
			Internal_FlushUploadBuffer();

		if (m_createInfo.platform->IsWindowResized())
		{
			m_createInfo.platform->SetWindowResized(false);
			m_self->device->WaitIdle();
			m_self->swapChain->RecreateSwapChain(static_cast<uint32_t>(windowWidth), static_cast<uint32_t>(windowHeight));
			m_self->swapChainImageLayouts.assign(m_self->swapChain->GetImageCount(), VK_IMAGE_LAYOUT_UNDEFINED);
		}

		m_self->inFlightFences.at(m_currentFrameIndex)->Wait();
		// Safe point: GPU has finished all work for this frame index; flush deferred deletions
		m_self->frameDeletionQueues[m_currentFrameIndex].Flush();
		FlushDescriptorSetUpdateRequests();

		const auto acquireResult = m_self->swapChain->AcquireNextImage(m_self->imageAvailableSemaphores.at(m_currentFrameIndex)->Handle());
		if (!acquireResult.has_value())
		{
			m_self->device->WaitIdle();
			m_self->swapChain->RecreateSwapChain(static_cast<uint32_t>(windowWidth), static_cast<uint32_t>(windowHeight));
			m_self->swapChainImageLayouts.assign(m_self->swapChain->GetImageCount(), VK_IMAGE_LAYOUT_UNDEFINED);
			return std::nullopt;
		}

		const uint32_t imageIndex = acquireResult.value();
		if (imageIndex < m_self->swapChainImageLayouts.size())
			m_self->swapChainImageLayouts[imageIndex] = VK_IMAGE_LAYOUT_UNDEFINED;

		m_self->inFlightFences.at(m_currentFrameIndex)->Reset();
		m_self->commandBuffers.at(m_currentFrameIndex)->Reset();
		m_self->commandBuffers.at(m_currentFrameIndex)->BeginRecording();

		return imageIndex;
	}

	void RenderingDevice::EndFrame(const uint32_t imageIndex)
	{
		TransitionSwapChainImage(imageIndex, ImageLayout::Present);

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

		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || m_createInfo.platform->IsWindowResized())
		{
			const auto [windowWidth, windowHeight] = m_createInfo.platform->GetWindowSize();
			m_createInfo.platform->SetWindowResized(false);
			if (windowWidth > 0 && windowHeight > 0)
			{
				m_self->device->WaitIdle();
				m_self->swapChain->RecreateSwapChain(static_cast<uint32_t>(windowWidth), static_cast<uint32_t>(windowHeight));
				m_self->swapChainImageLayouts.assign(m_self->swapChain->GetImageCount(), VK_IMAGE_LAYOUT_UNDEFINED);
			}
		}
		else if (presentResult != VK_SUCCESS)
			throw std::runtime_error("failed to present swap chain image!");

		m_currentFrameIndex = (m_currentFrameIndex + 1) % m_maxFramesInFlight;
	}

	void RenderingDevice::TransitionTexture(const Handle& textureHandle, const ImageLayout newLayout)
	{
		RenderingApi::Texture* texture = GetTexture(textureHandle);
		const VkImageLayout	   vkNewLayout = ToVkImageLayout(newLayout);
		const VkImageLayout	   vkOldLayout = texture->currentLayout;

		const bool isAttachmentSync = (vkNewLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
									   vkNewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
									   vkNewLayout == VK_IMAGE_LAYOUT_GENERAL);
		if (vkOldLayout == vkNewLayout && !isAttachmentSync)
			return;

		m_self->commandBuffers[m_currentFrameIndex]->TransitionImageLayout2(texture->value, texture->aspectMask, texture->mipLevels, vkOldLayout, vkNewLayout);
		texture->currentLayout = vkNewLayout;
	}

	void RenderingDevice::CopyTexture(const Handle& src, const Handle& dst)
	{
		RenderingApi::Texture* srcTex = GetTexture(src);
		RenderingApi::Texture* dstTex = GetTexture(dst);

		TransitionTexture(src, ImageLayout::TransferSrc);
		TransitionTexture(dst, ImageLayout::TransferDst);

		const uint32_t	  width = std::min(srcTex->width, dstTex->width);
		const uint32_t	  height = std::min(srcTex->height, dstTex->height);
		const VkImageCopy region = RenderingApi::CommandBuffer::MakeFullImageCopyRegion(srcTex->aspectMask, dstTex->aspectMask, width, height);

		m_self->commandBuffers[m_currentFrameIndex]->CopyImage(
			srcTex->value, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstTex->value, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			region
		);

		TransitionTexture(dst, ImageLayout::ShaderReadOnly);
	}

	void RenderingDevice::TransitionSwapChainImage(const uint32_t imageIndex, const ImageLayout newLayout)
	{
		const VkImageLayout vkNewLayout = ToVkImageLayout(newLayout);
		const VkImageLayout vkOldLayout = imageIndex < m_self->swapChainImageLayouts.size()
											  ? m_self->swapChainImageLayouts[imageIndex]
											  : VK_IMAGE_LAYOUT_UNDEFINED;

		if (vkOldLayout == vkNewLayout && vkNewLayout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			return;

		m_self->commandBuffers[m_currentFrameIndex]->TransitionImageLayout2(
			m_self->swapChain->GetImage(imageIndex), VK_IMAGE_ASPECT_COLOR_BIT, 1, vkOldLayout, vkNewLayout
		);

		if (imageIndex < m_self->swapChainImageLayouts.size())
			m_self->swapChainImageLayouts[imageIndex] = vkNewLayout;
	}

	void RenderingDevice::BeginRenderPass(const RenderPassDescriptor& desc)
	{
		const auto cmd = m_self->commandBuffers[m_currentFrameIndex].get();
		const auto swapExtent = m_self->swapChain->GetExtent();
		const VkExtent2D extent = {
			desc.width > 0 ? desc.width : swapExtent.width,
			desc.height > 0 ? desc.height : swapExtent.height
		};

		// Automatically transition all attachments into their required layouts before vkCmdBeginRendering
		for (const auto& colorDesc : desc.colorAttachments)
		{
			if (colorDesc.useSwapChainImage)
				TransitionSwapChainImage(colorDesc.swapChainImageIndex, ImageLayout::ColorAttachment);
			else if (colorDesc.target.IsValid())
				TransitionTexture(colorDesc.target, ImageLayout::ColorAttachment);

			if (colorDesc.resolveToSwapChain)
				TransitionSwapChainImage(colorDesc.swapChainImageIndex, ImageLayout::ColorAttachment);
			else if (colorDesc.resolveTarget.IsValid())
				TransitionTexture(colorDesc.resolveTarget, ImageLayout::ColorAttachment);
		}

		if (desc.depthAttachment.has_value() && desc.depthAttachment->target.IsValid())
			TransitionTexture(desc.depthAttachment->target, ImageLayout::DepthStencilAttachment);

		FixedArray<VkRenderingAttachmentInfo, RenderPassDescriptor::MaxColorAttachments> vkColorAttachments;

		for (const auto& colorDesc : desc.colorAttachments)
		{
			VkRenderingAttachmentInfo attachment{};
			attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachment.loadOp = ToVkLoadOp(colorDesc.loadOp);
			attachment.storeOp = ToVkStoreOp(colorDesc.storeOp);
			attachment.clearValue.color = {{colorDesc.clearColor[0], colorDesc.clearColor[1], colorDesc.clearColor[2], colorDesc.clearColor[3]}};

			if (colorDesc.useSwapChainImage)
			{
				attachment.imageView = m_self->swapChain->GetImageView(colorDesc.swapChainImageIndex);
			}
			else
			{
				attachment.imageView = GetTexture(colorDesc.target)->defaultView;
			}

			if (colorDesc.resolveToSwapChain)
			{
				attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
				attachment.resolveImageView = m_self->swapChain->GetImageView(colorDesc.swapChainImageIndex);
				attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
			else if (colorDesc.resolveTarget.IsValid())
			{
				attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
				attachment.resolveImageView = GetTexture(colorDesc.resolveTarget)->defaultView;
				attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
			else
			{
				attachment.resolveMode = VK_RESOLVE_MODE_NONE;
			}

			vkColorAttachments.Push(attachment);
		}

		VkRenderingAttachmentInfo vkDepthAttachment{};
		if (desc.depthAttachment.has_value())
		{
			const auto& depthDesc = desc.depthAttachment.value();
			vkDepthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			vkDepthAttachment.imageView = GetTexture(depthDesc.target)->defaultView;
			vkDepthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			vkDepthAttachment.loadOp = ToVkLoadOp(depthDesc.loadOp);
			vkDepthAttachment.storeOp = ToVkStoreOp(depthDesc.storeOp);
			vkDepthAttachment.clearValue.depthStencil = {depthDesc.clearDepth, depthDesc.clearStencil};
		}

		VkRenderingInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea = {{0, 0}, extent};
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = static_cast<uint32_t>(vkColorAttachments.size());
		renderingInfo.pColorAttachments = vkColorAttachments.empty() ? nullptr : vkColorAttachments.data();
		renderingInfo.pDepthAttachment = desc.depthAttachment.has_value() ? &vkDepthAttachment : nullptr;

		cmd->BeginRendering(renderingInfo);
		cmd->SetViewport(0, 0, extent, 0.0f, 1.0f);
		cmd->SetScissor(0, 0, extent);
	}

	void RenderingDevice::EndRenderPass()
	{
		m_self->commandBuffers[m_currentFrameIndex]->EndRendering();
	}

	void RenderingDevice::ExecuteDrawStream(const DrawStreamView& drawStream)
	{
		const auto cmd = m_self->commandBuffers[m_currentFrameIndex].get();

		Handle												  lastPipeline{};
		std::string											  lastLayoutName;
		VkPipelineLayout									  currentVkLayout = VK_NULL_HANDLE;
		std::array<Handle, DrawPacket::MaxDescriptorSets>	  lastDescriptorSets{};
		Handle												  lastVertexBuffer{};
		Handle												  lastIndexBuffer{};

		for (const DrawPacket& packet : drawStream)
		{
			if (lastPipeline != packet.pipeline)
			{
				const auto* pipeline = GetPipeline(packet.pipeline);
				cmd->BindPipeline(pipeline->pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
				lastPipeline = packet.pipeline;
			}

			if (lastLayoutName != packet.pipelineLayoutName)
			{
				currentVkLayout = GetPipelineLayout(packet.pipelineLayoutName)->Handle();
				lastLayoutName = packet.pipelineLayoutName;
				lastDescriptorSets.fill({});
			}

			for (uint32_t setIdx = 0; setIdx < packet.descriptorSetCount; ++setIdx)
			{
				const Handle setHandle = packet.descriptorSets[setIdx];
				if (!setHandle.IsValid() || lastDescriptorSets[setIdx] == setHandle)
					continue;

				const auto* descSet = GetDescriptorSet(setHandle);
				cmd->BindDescriptorSets(currentVkLayout, setIdx, 1, &descSet->value, 0, nullptr);
				lastDescriptorSets[setIdx] = setHandle;
			}

			if (packet.pushConstantSize > 0)
			{
				cmd->PushConstants(currentVkLayout, ToVkShaderStageFlags(packet.pushConstantStages), 0, packet.pushConstantSize, packet.pushConstantData.data());
			}

			if (lastVertexBuffer != packet.vertexBuffer)
			{
				const auto*			   vertexBuffer = GetBuffer(packet.vertexBuffer);
				const VkBuffer		   vertexBuffers[] = {vertexBuffer->value};
				constexpr VkDeviceSize offsets[] = {0};
				cmd->BindVertexBuffers(0, 1, vertexBuffers, offsets);
				lastVertexBuffer = packet.vertexBuffer;
			}

			if (lastIndexBuffer != packet.indexBuffer)
			{
				const auto* indexBuffer = GetBuffer(packet.indexBuffer);
				cmd->BindIndexBuffer(indexBuffer->value, 0, VK_INDEX_TYPE_UINT32);
				lastIndexBuffer = packet.indexBuffer;
			}

			cmd->DrawIndexed(packet.indexCount, packet.instanceCount, 0, 0, 0);
		}
	}

	void RenderingDevice::DrawFullscreenTriangle(const Handle& pipeline, const std::string_view layoutName, const std::span<const Handle> descriptorSets)
	{
		const auto			   cmd = m_self->commandBuffers[m_currentFrameIndex].get();
		const auto*			   p = GetPipeline(pipeline);
		const VkPipelineLayout vkLayout = GetPipelineLayout(std::string(layoutName))->Handle();

		cmd->BindPipeline(p->pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
		for (uint32_t i = 0; i < static_cast<uint32_t>(descriptorSets.size()); ++i)
		{
			if (!descriptorSets[i].IsValid())
				continue;
			const auto* descSet = GetDescriptorSet(descriptorSets[i]);
			cmd->BindDescriptorSets(vkLayout, i, 1, &descSet->value, 0, nullptr);
		}
		cmd->Draw(3, 1, 0, 0);
	}

	void RenderingDevice::BindComputePipeline(const Handle& pipeline)
	{
		const auto* p = GetPipeline(pipeline);
		m_self->commandBuffers[m_currentFrameIndex]->BindPipeline(p->pipeline, VK_PIPELINE_BIND_POINT_COMPUTE);
	}

	void RenderingDevice::BindComputeDescriptorSets(const std::string_view layoutName, const uint32_t firstSet, const std::span<const Handle> sets)
	{
		const VkPipelineLayout layout = GetPipelineLayout(std::string(layoutName))->Handle();
		auto*				   cmd = m_self->commandBuffers[m_currentFrameIndex].get();
		for (uint32_t i = 0; i < static_cast<uint32_t>(sets.size()); ++i)
		{
			if (!sets[i].IsValid())
				continue;
			const auto* descSet = GetDescriptorSet(sets[i]);
			cmd->BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, layout, firstSet + i, 1, &descSet->value, 0, nullptr);
		}
	}

	void RenderingDevice::Dispatch(const uint32_t groupX, const uint32_t groupY, const uint32_t groupZ)
	{
		m_self->commandBuffers[m_currentFrameIndex]->Dispatch(groupX, groupY, groupZ);
	}

	void RenderingDevice::ComputeBarrier()
	{
		m_self->commandBuffers[m_currentFrameIndex]->ComputeBarrier();
	}

	void RenderingDevice::PushComputeConstants(const std::string_view layoutName, const void* data, const uint32_t size)
	{
		const VkPipelineLayout layout = GetPipelineLayout(std::string(layoutName))->Handle();
		m_self->commandBuffers[m_currentFrameIndex]->PushConstants(layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, size, data);
	}

	void RenderingDevice::WaitIdle() const
	{
		m_self->device->WaitIdle();
	}

	uint32_t RenderingDevice::GetCurrentFrameIndex() const
	{
		return m_currentFrameIndex;
	}

	uint32_t RenderingDevice::GetMaxFramesInFlight() const
	{
		return m_maxFramesInFlight;
	}

	float RenderingDevice::GetAspectRatio() const
	{
		return m_self->swapChain->GetAspectRatio();
	}

	std::pair<uint32_t, uint32_t> RenderingDevice::GetSwapChainExtent() const
	{
		const VkExtent2D ext = m_self->swapChain->GetExtent();
		return {ext.width, ext.height};
	}

	uint64_t RenderingDevice::GetSwapChainGeneration() const
	{
		return m_self->swapChain->GetGeneration();
	}

	bool RenderingDevice::IsMsaaEnabled() const
	{
		return m_self->device->GetMaxMsaaSamples() > VK_SAMPLE_COUNT_1_BIT;
	}

	bool RenderingDevice::IsRayQuerySupported() const
	{
		return m_self->device->IsRayQuerySupported();
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

	RenderingApi::SwapChain* RenderingDevice::GetSwapChain() const
	{
		return m_self->swapChain.get();
	}
}
