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

#include "DescriptorSetUpdateRequest.hpp"
#include "Handle.hpp"
#include "HandleRegistry.hpp"
#include "SamplerDescription.hpp"
#include "TextureDescription.hpp"
#include "Types.hpp"

#include <Module_Base/EBufferingMode.hpp>
#include <memory>
#include <optional>
#include <unordered_map>

namespace BetterThanNothing
{
	struct RenderingDeviceInternals;

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
		class DescriptorSetLayout;
		struct DescriptorSetLayoutCreateInfo;
		struct Sampler;
		struct Buffer;
		struct Texture;
		struct DescriptorSet;
		class CommandBuffer;
		class Semaphore;
		class Fence;
	}

	class StagingBuffer;
	struct BufferDescription;
	struct TextureFromDataDescription;

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

	private:
		void Internal_RegisterDescriptorLayout(DescriptorSetLayoutType key, VkDescriptorType type, VkShaderStageFlags stageFlags);

	public:
		Handle				   GetOrCreateTextureSampler(const SamplerDescription& desc);
		void				   DestroyTextureSampler(const Handle& handle);
		RenderingApi::Sampler* GetSampler(const Handle& handle);

		Handle				  CreateBuffer(const BufferDescription& desc);
		Handle				  CreateUniformBuffer(size_t size);
		Handle				  CreateStorageBuffer(size_t size);
		Handle				  CreateVertexBuffer(const void* data, size_t size);
		Handle				  CreateIndexBuffer(const void* data, size_t size);
		bool				  StageDataToBuffer(const Handle& dstBufferHandle, const void* data, size_t size);
		bool				  CopyDataToBuffer(const Handle& dstBufferHandle, const void* data, size_t size);
		void				  DestroyBuffer(const Handle& handle);
		RenderingApi::Buffer* GetBuffer(const Handle& handle);
		void*				  GetBufferData(const Handle& handle);

	private:
		bool Internal_CreateTextureImage(const TextureImageDescription& desc);
		bool Internal_CreateTextureImageView(const TextureImageViewDescription& desc);
		void Internal_CopyDataToTextureImage(const CopyDataToTextureImageDescription& desc);
		bool Internal_CreateTextureMipmaps(const Handle& destination);

	public:
		Handle				   CreateTextureFromData(const TextureFromDataDescription& desc);
		void				   DestroyTexture(const Handle& handle);
		RenderingApi::Texture* GetTexture(const Handle& handle);

	private:
		bool Internal_CreateDescriptorSet(const DescriptorSetLayoutType& type, RenderingApi::DescriptorSet* newDescriptorSet) const;

	public:
		[[nodiscard]] Handle CreateDescriptorSet(DescriptorSetLayoutType type);
		[[nodiscard]] Handle CreateUniformBufferDescriptorSet(const Handle& uniformBufferHandle, size_t size, DescriptorSetLayoutType layoutType);
		[[nodiscard]] Handle CreateStorageBufferDescriptorSet(const Handle& uniformBufferHandle, size_t size, DescriptorSetLayoutType layoutType);
		[[nodiscard]] Handle		 CreateTextureDescriptorSet(const Handle& textureHandle, const Handle& samplerHandle);
		void						 RequestDescriptorSetUpdate(const DescriptorSetUpdateRequest& request);
		void						 FlushDescriptorSetUpdateRequests();
		void						 DestroyDescriptorSet(const Handle& handle);
		RenderingApi::DescriptorSet* GetDescriptorSet(const Handle& handle);

	private:
		void Internal_CreateCommandBuffers();
		void Internal_CreateSyncObjects();

	public:
		std::optional<uint32_t> BeginFrame();
		void					EndFrame(uint32_t imageIndex);

		void WaitIdle() const;

		/// @deprecated Will be removed in the future.
		[[nodiscard]] uint32_t GetCurrentFrameIndex() const;
		/// @deprecated Will be removed in the future.
		[[nodiscard]] RenderingApi::CommandBuffer* GetCurrentCommandBuffer() const;
		/// @deprecated Will be removed in the future.
		[[nodiscard]] RenderingApi::Instance* GetInstance() const;
		/// @deprecated Will be removed in the future.
		[[nodiscard]] RenderingApi::Device* GetDevice() const;
		/// @deprecated Will be removed in the future.
		[[nodiscard]] RenderingApi::Queue* GetGraphicsQueue() const;
		/// @deprecated Will be removed in the future.
		[[nodiscard]] RenderingApi::Allocator* GetAllocator() const;
		/// @deprecated Will be removed in the future.
		[[nodiscard]] RenderingApi::SwapChain* GetSwapChain() const;

		[[nodiscard]] auto GetMaxFramesInFlight() const -> uint32_t;
		[[nodiscard]] auto GetAspectRatio() const -> float;

		/// @deprecated Will be removed in the future.
		[[nodiscard]] RenderingApi::DescriptorPool*		 GetDescriptorPool() const;
		[[nodiscard]] RenderingApi::DescriptorSetLayout* GetDescriptorSetLayout(DescriptorSetLayoutType key) const;

	private:
		RenderingDeviceCreateInfo				  m_createInfo;
		std::unique_ptr<RenderingDeviceInternals> m_self;

		std::unique_ptr<RenderingApi::DescriptorPool> m_descriptorPool;

		std::vector<std::unique_ptr<RenderingApi::CommandBuffer>> m_commandBuffers;
		std::vector<std::unique_ptr<RenderingApi::Semaphore>>	  m_imageAvailableSemaphores;
		std::vector<std::unique_ptr<RenderingApi::Fence>>		  m_inFlightFences;

		std::unique_ptr<StagingBuffer> m_stagingBuffer;

		std::unordered_map<DescriptorSetLayoutType, std::unique_ptr<RenderingApi::DescriptorSetLayout>> m_descriptorLayouts;

		HandleRegistry<RenderingApi::Buffer, HandleType::Buffer>			   m_buffers;
		HandleRegistry<RenderingApi::Texture, HandleType::Texture>			   m_textures;
		HandleRegistry<RenderingApi::Sampler, HandleType::Sampler>			   m_samplers;
		HandleRegistry<RenderingApi::DescriptorSet, HandleType::DescriptorSet> m_descriptors;

		std::unordered_map<SamplerDescription, Handle, SamplerDescriptionHash, SamplerDescriptionEqual> m_samplersLUT;
		std::vector<DescriptorSetUpdateRequest>															m_descriptorsUpdateRequests;
		std::vector<VkWriteDescriptorSet>																m_descriptorsWrites;

		uint32_t m_maxFramesInFlight;
		uint32_t m_currentFrameIndex = 0;
	};
};
