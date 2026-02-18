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
	namespace RenderingApi
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
	}

	struct RenderingDeviceCreateInfo
	{
		EBufferingMode bufferingMode = EBufferingMode::Double;
	};

	class RenderingDevice
	{
	public:
		explicit RenderingDevice(const RenderingDeviceCreateInfo& createInfo);
		~RenderingDevice();

		[[nodiscard]] static std::unique_ptr<RenderingDevice> Create(const RenderingDeviceCreateInfo& createInfo);

		/// @deprecated Will be removed in the future.
		[[nodiscard]] RenderingApi::Instance*		GetInstance() const;
		/// @deprecated Will be removed in the future.
		[[nodiscard]] RenderingApi::Surface*		GetSurface() const;
		/// @deprecated Will be removed in the future.
		[[nodiscard]] RenderingApi::Device*			GetDevice() const;
		/// @deprecated Will be removed in the future.
		[[nodiscard]] RenderingApi::Queue*			GetGraphicsQueue() const;
		/// @deprecated Will be removed in the future.
		[[nodiscard]] RenderingApi::Queue*			GetPresentQueue() const;
		/// @deprecated Will be removed in the future.
		[[nodiscard]] RenderingApi::Allocator*		GetAllocator() const;
		/// @deprecated Will be removed in the future.
		[[nodiscard]] RenderingApi::SwapChain*		GetSwapChain() const;
		/// @deprecated Will be removed in the future.
		[[nodiscard]] RenderingApi::CommandPool*	GetCommandPool() const;
		/// @deprecated Will be removed in the future.
		[[nodiscard]] RenderingApi::DescriptorPool* GetDescriptorPool() const;
		/// @deprecated Will be removed in the future.
		[[nodiscard]] RenderingApi::Pipeline*		GetDefaultPipeline() const;
		/// @deprecated Will be removed in the future.
		[[nodiscard]] uint32_t						GetMaxFramesInFlight() const;

	private:
		RenderingDeviceCreateInfo					  m_createInfo;
		std::unique_ptr<RenderingApi::Instance>		  m_instance;
		std::unique_ptr<RenderingApi::Surface>		  m_surface;
		std::unique_ptr<RenderingApi::Device>		  m_device;
		std::unique_ptr<RenderingApi::Queue>		  m_graphicsQueue;
		std::unique_ptr<RenderingApi::Queue>		  m_presentQueue;
		std::unique_ptr<RenderingApi::Allocator>	  m_allocator;
		std::unique_ptr<RenderingApi::SwapChain>	  m_swapChain;
		std::unique_ptr<RenderingApi::CommandPool>	  m_commandPool;
		std::unique_ptr<RenderingApi::DescriptorPool> m_descriptorPool;
		std::unique_ptr<RenderingApi::Pipeline>		  m_defaultPipeline;
		uint32_t									  m_maxFramesInFlight;
	};
};
