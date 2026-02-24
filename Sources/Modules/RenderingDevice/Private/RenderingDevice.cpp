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

#include "RenderingDevice/BufferRegistry.hpp"
#include "RenderingDevice/Handle.hpp"
#include "RenderingDevice/SamplerRegistry.hpp"

#include <Module_Base/Logger.hpp>
#include <Module_Platform/Manager/WindowManager.hpp>
#include <Module_Platform/Window.hpp>
#include <RenderingApi.hpp>

namespace BetterThanNothing
{
	RenderingDevice::RenderingDevice(const RenderingDeviceCreateInfo& createInfo) : m_createInfo(createInfo)
	{
		const auto& window = WindowManager::GetWindow();
		const auto [windowWidth, windowHeight] = WindowManager::GetWindow()->GetFramebufferSize();

		m_instance = RenderingApi::Instance::Create({.enableValidation = true});
		m_surface = RenderingApi::Surface::Create({window->Handle(), m_instance.get()});
		m_device = RenderingApi::Device::Create({m_instance.get(), m_surface.get()});
		m_graphicsQueue = m_device->CreateGraphicsQueue();
		m_presentQueue = m_device->CreatePresentQueue();
		m_allocator = RenderingApi::Allocator::Create({m_instance.get(), m_device.get()});

		m_commandPool = RenderingApi::CommandPool::Create({m_device.get(), m_graphicsQueue.get()});

		m_swapChain = RenderingApi::SwapChain::Create({
			m_surface.get(),
			m_device.get(),
			m_allocator.get(),
			windowWidth,
			windowHeight,
			createInfo.bufferingMode,
		});

		m_maxFramesInFlight = m_swapChain->GetImageCount();

		Logger::Log<INFO>("Vendor: {}", m_device->GetVendorName());
		Logger::Log<INFO>("Device: {}", m_device->GetDeviceName());
		Logger::Log<INFO>("API Version: {}", m_device->GetApiVersion());
		Logger::Log<INFO>("Buffering mode: {}", static_cast<uint8_t>(createInfo.bufferingMode));
		Logger::Log<INFO>("Max frame in-flight: {}", m_maxFramesInFlight);
		Logger::Log<INFO>("VSync: {}", createInfo.bufferingMode != EBufferingMode::Immediate ? "enabled" : "disabled");
		Logger::Log<INFO>("Anisotropy: {}x", m_device->GetMaxAnisotropy());
		Logger::Log<INFO>("Msaa: {}x", static_cast<uint32_t>(m_device->GetMaxMsaaSamples()));

		m_descriptorPool = RenderingApi::DescriptorPool::Builder(m_device.get())
							   .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_maxFramesInFlight)
							   .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
							   .AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, m_maxFramesInFlight)
							   .Build();

		RegisterDescriptorLayout(EDescriptorLayoutKey::GlobalUniforms, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		RegisterDescriptorLayout(EDescriptorLayoutKey::Textures, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		RegisterDescriptorLayout(EDescriptorLayoutKey::PointLights, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);

		m_samplerRegistry = SamplerRegistry::Create(m_device.get());
		m_bufferRegistry = BufferRegistry::Create({m_device.get(), m_commandPool.get(), m_graphicsQueue.get(), m_allocator.get()});
	}

	RenderingDevice::~RenderingDevice() {}

	void RenderingDevice::RegisterDescriptorLayout(const EDescriptorLayoutKey key, const VkDescriptorType type, const VkShaderStageFlags stageFlags)
	{
		m_layouts[key] = RenderingApi::DescriptorSetLayout::Create({m_device.get(), type, 1, stageFlags});
	}

	std::unique_ptr<RenderingDevice> RenderingDevice::Create(const RenderingDeviceCreateInfo& createInfo)
	{
		return std::make_unique<RenderingDevice>(createInfo);
	}

	SamplerHandle RenderingDevice::GetOrCreateTextureSampler(const SamplerDescription& desc) const
	{
		return m_samplerRegistry->GetOrCreateSampler(desc);
	}

	void RenderingDevice::DestroyTextureSampler(const SamplerHandle handle) const
	{
		m_samplerRegistry->DestroySampler(handle);
	}

	BufferHandle RenderingDevice::CreateBuffer(const BufferDescription& desc) const
	{
		return m_bufferRegistry->CreateBuffer(desc);
	}

	BufferHandle RenderingDevice::CreateVertexBuffer(const void* data, const size_t size) const
	{
		return m_bufferRegistry->CreateVertexBuffer(data, size);
	}

	BufferHandle RenderingDevice::CreateIndexBuffer(const void* data, const size_t size) const
	{
		return m_bufferRegistry->CreateIndexBuffer(data, size);
	}

	bool RenderingDevice::CopyDataToBuffer(const BufferHandle dstBufferHandle, const void* data, const size_t size) const
	{
		return m_bufferRegistry->CopyDataToBuffer(dstBufferHandle, data, size);
	}

	void RenderingDevice::DestroyBuffer(const BufferHandle handle) const
	{
		return m_bufferRegistry->DestroyBuffer(handle);
	}

	RenderingApi::Buffer* RenderingDevice::GetBuffer(const BufferHandle handle) const
	{
		return m_bufferRegistry->GetBuffer(handle);
	}

	RenderingApi::Instance* RenderingDevice::GetInstance() const
	{
		return m_instance.get();
	}

	RenderingApi::Surface* RenderingDevice::GetSurface() const
	{
		return m_surface.get();
	}

	RenderingApi::Device* RenderingDevice::GetDevice() const
	{
		return m_device.get();
	}

	RenderingApi::Queue* RenderingDevice::GetGraphicsQueue() const
	{
		return m_graphicsQueue.get();
	}

	RenderingApi::Queue* RenderingDevice::GetPresentQueue() const
	{
		return m_presentQueue.get();
	}

	RenderingApi::Allocator* RenderingDevice::GetAllocator() const
	{
		return m_allocator.get();
	}

	RenderingApi::SwapChain* RenderingDevice::GetSwapChain() const
	{
		return m_swapChain.get();
	}

	RenderingApi::CommandPool* RenderingDevice::GetCommandPool() const
	{
		return m_commandPool.get();
	}

	RenderingApi::Pipeline* RenderingDevice::GetDefaultPipeline() const
	{
		return m_defaultPipeline.get();
	}

	uint32_t RenderingDevice::GetMaxFramesInFlight() const
	{
		return m_maxFramesInFlight;
	}

	RenderingApi::DescriptorPool* RenderingDevice::GetDescriptorPool() const
	{
		return m_descriptorPool.get();
	}

	RenderingApi::DescriptorSetLayout* RenderingDevice::GetDescriptorSetLayout(EDescriptorLayoutKey key) const
	{
		if (!m_layouts.contains(key))
			throw std::runtime_error("Cannot find descriptorSetLayout");
		return m_layouts.at(key).get();
	}
}
