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

#pragma once

#include <RenderingApi/Core/Instance.hpp>
#include <RenderingApi/Core/Surface.hpp>
#include <RenderingApi/Core/Device.hpp>
#include <RenderingApi/Core/Queue.hpp>
#include <RenderingApi/Core/Allocator.hpp>
#include <RenderingApi/Core/CommandPool.hpp>
#include <RenderingApi/Core/SwapChain.hpp>
#include <RenderingApi/Core/DescriptorPool.hpp>
#include <RenderingApi/Core/CommandBuffer.hpp>
#include <RenderingApi/Core/Semaphore.hpp>
#include <RenderingApi/Core/Fence.hpp>

namespace BetterThanNothing
{
	struct RenderingDeviceInternals
	{
		std::unique_ptr<RenderingApi::Instance>		  instance;
		std::unique_ptr<RenderingApi::Surface>		  surface;
		std::unique_ptr<RenderingApi::Device>		  device;
		std::unique_ptr<RenderingApi::Queue>		  graphicsQueue;
		std::unique_ptr<RenderingApi::Queue>		  presentQueue;
		std::unique_ptr<RenderingApi::Allocator>	  allocator;
		std::unique_ptr<RenderingApi::CommandPool>	  commandPool;
		std::unique_ptr<RenderingApi::SwapChain>	  swapChain;
		std::unique_ptr<RenderingApi::DescriptorPool> descriptorPool;

		std::vector<std::unique_ptr<RenderingApi::CommandBuffer>> commandBuffers;
		std::vector<std::unique_ptr<RenderingApi::Semaphore>>	  imageAvailableSemaphores;
		std::vector<std::unique_ptr<RenderingApi::Fence>>		  inFlightFences;

		std::unordered_map<DescriptorSetLayoutType, std::unique_ptr<RenderingApi::DescriptorSetLayout>> descriptorLayouts;

		std::unique_ptr<StagingBuffer> stagingBuffer;

		HandleRegistry<RenderingApi::Buffer, HandleType::Buffer>			   buffers;
		HandleRegistry<RenderingApi::Texture, HandleType::Texture>			   textures;
		HandleRegistry<RenderingApi::Sampler, HandleType::Sampler>			   samplers;
		HandleRegistry<RenderingApi::DescriptorSet, HandleType::DescriptorSet> descriptors;

		std::unordered_map<SamplerDescription, Handle, SamplerDescriptionHash, SamplerDescriptionEqual> samplersLUT;
		std::vector<DescriptorSetUpdateRequest>															descriptorsUpdateRequests;
		std::vector<VkWriteDescriptorSet>																descriptorsWrites;
	};
}
