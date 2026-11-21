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

#include "DeletionQueue.hpp"
#include "DescriptorAllocator.hpp"
#include "StagingBuffer.hpp"

#include <RenderingApi/Core/Allocator.hpp>
#include <RenderingApi/Core/CommandBuffer.hpp>
#include <RenderingApi/Core/CommandPool.hpp>
#include <RenderingApi/Core/DescriptorPool.hpp>
#include <RenderingApi/Core/DescriptorSetLayout.hpp>
#include <RenderingApi/Core/Device.hpp>
#include <RenderingApi/Core/Fence.hpp>
#include <RenderingApi/Core/Instance.hpp>
#include <RenderingApi/Core/PipelineLayout.hpp>
#include <RenderingApi/Core/Queue.hpp>
#include <RenderingApi/Core/Semaphore.hpp>
#include <RenderingApi/Core/Surface.hpp>
#include <RenderingApi/Core/SwapChain.hpp>
#include <RenderingApi/Resources/AccelerationStructure.hpp>
#include <RenderingApi/Resources/Buffer.hpp>
#include <RenderingApi/Resources/DescriptorSet.hpp>
#include <RenderingApi/Resources/Pipeline.hpp>
#include <RenderingApi/Resources/Sampler.hpp>
#include <RenderingApi/Resources/Shader.hpp>
#include <RenderingApi/Resources/Texture.hpp>
#include <RenderingDevice/DescriptorSetUpdateRequest.hpp>
#include <RenderingDevice/HandleRegistry.hpp>
#include <RenderingDevice/SamplerDescription.hpp>

namespace BetterThanNothing
{
	struct RenderingDeviceInternals
	{
		std::unique_ptr<RenderingApi::Instance>		   instance;
		std::unique_ptr<RenderingApi::Surface>		   surface;
		std::unique_ptr<RenderingApi::Device>		   device;
		std::unique_ptr<RenderingApi::Queue>		   graphicsQueue;
		std::unique_ptr<RenderingApi::Queue>		   presentQueue;
		std::unique_ptr<RenderingApi::Allocator>	   allocator;
		std::unique_ptr<RenderingApi::CommandPool>	   commandPool;
		std::unique_ptr<RenderingApi::SwapChain>	   swapChain;
		std::unique_ptr<DescriptorAllocator>		   descriptorAllocator;

		std::vector<std::unique_ptr<RenderingApi::CommandBuffer>> commandBuffers;
		std::vector<std::unique_ptr<RenderingApi::Semaphore>>	  imageAvailableSemaphores;
		std::vector<std::unique_ptr<RenderingApi::Fence>>		  inFlightFences;
		std::vector<DeletionQueue>								  frameDeletionQueues;
		std::vector<VkImageLayout>								  swapChainImageLayouts;

		std::unordered_map<std::string, std::unique_ptr<RenderingApi::DescriptorSetLayout>> descriptorLayouts;
		std::unordered_map<std::string, std::unique_ptr<RenderingApi::PipelineLayout>>		pipelineLayouts;

		std::unique_ptr<StagingBuffer>				 stagingBuffer;
		std::unique_ptr<RenderingApi::CommandBuffer> uploadCommandBuffer;
		uint32_t									 uploadBatchDepth = 0;
		bool										 uploadRecording = false;
		bool										 shuttingDown = false;

		HandleRegistry<RenderingApi::Buffer, HandleType::Buffer>			   buffers;
		HandleRegistry<RenderingApi::Texture, HandleType::Texture>			   textures;
		HandleRegistry<RenderingApi::Sampler, HandleType::Sampler>			   samplers;
		HandleRegistry<RenderingApi::DescriptorSet, HandleType::DescriptorSet> descriptors;
		HandleRegistry<RenderingApi::Shader, HandleType::Shader>							   shaders;
		HandleRegistry<RenderingApi::Pipeline, HandleType::Pipeline>						   pipelines;
		HandleRegistry<RenderingApi::AccelerationStructure, HandleType::AccelerationStructure> accelerationStructures;

		std::unordered_map<SamplerDescription, Handle, SamplerDescriptionHash, SamplerDescriptionEqual> samplersLUT;
		std::vector<DescriptorSetUpdateRequest>															descriptorsUpdateRequests;
		std::vector<VkWriteDescriptorSet>																descriptorsWrites;

		// AMD FidelityFX SDK v2.4.0 state (FSR 4.1.2 Upscaler + Frame Generation 4.0.2/3.1.6 + Denoiser 1.2.1 configuration)
		void*					 ffxUpscaleContext = nullptr;
		uint32_t				 ffxContextMaxRenderWidth = 0;
		uint32_t				 ffxContextMaxRenderHeight = 0;
		uint32_t				 ffxContextMaxUpscaleWidth = 0;
		uint32_t				 ffxContextMaxUpscaleHeight = 0;
		uint32_t				 ffxContextVersionIndex = 0;
		bool					 ffxInitialized = false;
		bool					 ffxUpscalerAvailable = false;
		std::vector<uint64_t>	 ffxUpscalerVersionIds;
		std::vector<std::string> ffxUpscalerVersionNames;
		float					 ffxLastVelocityFactor = -1.0f;

		void*					 ffxFrameGenContext = nullptr;
		uint32_t				 ffxFgMaxRenderWidth = 0;
		uint32_t				 ffxFgMaxRenderHeight = 0;
		uint32_t				 ffxFgDisplayWidth = 0;
		uint32_t				 ffxFgDisplayHeight = 0;
		uint32_t				 ffxFgVersionIndex = 0;
		bool					 ffxFrameGenAvailable = false;
		std::vector<uint64_t>	 ffxFrameGenVersionIds;
		std::vector<std::string> ffxFrameGenVersionNames;
		uint64_t				 ffxFrameGenFrameId = 1;
	};
}
