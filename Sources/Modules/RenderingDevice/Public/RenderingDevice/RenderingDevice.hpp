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

#include <Module_Base/EBufferingMode.hpp>
#include <memory>

namespace BetterThanNothing
{
	class Instance;
	class Surface;
	class Device;
	class Queue;
	class Allocator;
	class SwapChain;
	class CommandPool;
	class Pipeline;
	class DescriptorPool;

	struct RenderingDeviceCreateInfo
	{
		union
		{
			EBufferingMode bufferingMode = EBufferingMode::Double;
			uint32_t	   maxFramesInFlight;
		};
	};

	class RenderingDevice
	{
	public:
		explicit RenderingDevice(const RenderingDeviceCreateInfo& createInfo);
		~RenderingDevice();

		[[nodiscard]] static std::unique_ptr<RenderingDevice> Create(const RenderingDeviceCreateInfo& createInfo);

		[[nodiscard]] Instance*		  GetInstance() const;
		[[nodiscard]] Surface*		  GetSurface() const;
		[[nodiscard]] Device*		  GetDevice() const;
		[[nodiscard]] Queue*		  GetGraphicsQueue() const;
		[[nodiscard]] Queue*		  GetPresentQueue() const;
		[[nodiscard]] Allocator*	  GetAllocator() const;
		[[nodiscard]] SwapChain*	  GetSwapChain() const;
		[[nodiscard]] CommandPool*	  GetCommandPool() const;
		[[nodiscard]] DescriptorPool* GetDescriptorPool() const;
		[[nodiscard]] Pipeline*		  GetDefaultPipeline() const;

	private:
		RenderingDeviceCreateInfo		m_createInfo;
		std::unique_ptr<Instance>		m_instance;
		std::unique_ptr<Surface>		m_surface;
		std::unique_ptr<Device>			m_device;
		std::unique_ptr<Queue>			m_graphicsQueue;
		std::unique_ptr<Queue>			m_presentQueue;
		std::unique_ptr<Allocator>		m_allocator;
		std::unique_ptr<SwapChain>		m_swapChain;
		std::unique_ptr<CommandPool>	m_commandPool;
		std::unique_ptr<DescriptorPool> m_descriptorPool;
		std::unique_ptr<Pipeline>		m_defaultPipeline;
	};
};
