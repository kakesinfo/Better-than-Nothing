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

#include "RenderingDevice/RenderingDevice.hpp"

#include <Module_Base/Logger.hpp>
#include <Module_Platform/Manager/WindowManager.hpp>
#include <Module_Platform/Window.hpp>
#include <RenderingApi/Allocator.hpp>
#include <RenderingApi/CommandPool.hpp>
#include <RenderingApi/Device.hpp>
#include <RenderingApi/Instance.hpp>
#include <RenderingApi/Surface.hpp>
#include <RenderingApi/SwapChain.hpp>
#include <RenderingApi/Queue.hpp>
#include <RenderingApi/DescriptorPool.hpp>
#include <RenderingApi/Pipeline.hpp>

namespace BetterThanNothing
{
	RenderingDevice::RenderingDevice(const RenderingDeviceCreateInfo& createInfo) : m_createInfo(createInfo)
	{
		const auto& window = WindowManager::GetWindow();
		const auto [windowWidth, windowHeight] = WindowManager::GetWindow()->GetFramebufferSize();

		Logger::Log<INFO>("Buffering mode: {}", createInfo.maxFramesInFlight);
		Logger::Log<INFO>("VSync: {}", createInfo.bufferingMode == EBufferingMode::Double ? "enabled" : "disabled");

		m_instance = Instance::Create({.enableValidation = true});
		m_surface = Surface::Create({window->Handle(), m_instance.get()});
		m_device = Device::Create({m_instance.get(), m_surface.get()});
		m_graphicsQueue = m_device->CreateGraphicsQueue();
		m_presentQueue = m_device->CreatePresentQueue();
		m_allocator = Allocator::Create({m_instance.get(), m_device.get()});

		Logger::Log<INFO>("Vendor: {}", m_device->GetVendorName());
		Logger::Log<INFO>("Device: {}", m_device->GetDeviceName());
		Logger::Log<INFO>("API Version: {}", m_device->GetApiVersion());
		Logger::Log<INFO>("Anisotropy: {}x", m_device->GetMaxAnisotropy());
		Logger::Log<INFO>("Msaa: {}x", static_cast<uint32_t>(m_device->GetMaxMsaaSamples()));

		m_commandPool = CommandPool::Create({m_device.get(), m_graphicsQueue.get()});

		m_swapChain = SwapChain::Create({
			m_surface.get(),
			m_device.get(),
			m_allocator.get(),
			m_commandPool.get(),
			m_graphicsQueue.get(),
			windowWidth,
			windowHeight,
			createInfo.bufferingMode,
		});
	}

	RenderingDevice::~RenderingDevice() {}

	std::unique_ptr<RenderingDevice> RenderingDevice::Create(const RenderingDeviceCreateInfo& createInfo)
	{
		return std::make_unique<RenderingDevice>(createInfo);
	}

	Instance* RenderingDevice::GetInstance() const
	{
		return m_instance.get();
	}

	Surface* RenderingDevice::GetSurface() const
	{
		return m_surface.get();
	}

	Device* RenderingDevice::GetDevice() const
	{
		return m_device.get();
	}

	Queue* RenderingDevice::GetGraphicsQueue() const
	{
		return m_graphicsQueue.get();
	}

	Queue* RenderingDevice::GetPresentQueue() const
	{
		return m_presentQueue.get();
	}

	Allocator* RenderingDevice::GetAllocator() const
	{
		return m_allocator.get();
	}

	SwapChain* RenderingDevice::GetSwapChain() const
	{
		return m_swapChain.get();
	}

	CommandPool* RenderingDevice::GetCommandPool() const
	{
		return m_commandPool.get();
	}

	DescriptorPool* RenderingDevice::GetDescriptorPool() const
	{
		return m_descriptorPool.get();
	}

	Pipeline* RenderingDevice::GetDefaultPipeline() const
	{
		return m_defaultPipeline.get();
	}
}
