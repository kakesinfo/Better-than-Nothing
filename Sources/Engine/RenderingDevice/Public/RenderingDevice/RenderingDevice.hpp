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

#include <Common/EBufferingMode.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>

namespace BetterThanNothing
{
	struct ShaderDescription;
	struct RenderingDeviceInternals;

	class Platform;

	namespace RenderingApi
	{
		class Instance;
		class Surface;
		class Device;
		class Queue;
		class Allocator;
		class SwapChain;
		class CommandPool;
		class CommandBuffer;
		class DescriptorPool;
		class DescriptorSetLayout;
		struct Buffer;
		struct Sampler;
		struct Texture;
		struct DescriptorSet;
		struct Pipeline;
		class PipelineLayout;
	}

	class StagingBuffer;
	struct BufferDescription;
	struct TextureFromDataDescription;
	struct PipelineDescription;
	class PipelineLayoutBlueprint;

	struct RenderingDeviceCreateInfo
	{
		Platform*	   platform;
		EBufferingMode bufferingMode = EBufferingMode::Double;
	};

	class RenderingDevice
	{
	public:
		explicit RenderingDevice(const RenderingDeviceCreateInfo& createInfo);
		~RenderingDevice();

		[[nodiscard]] static std::unique_ptr<RenderingDevice> Create(const RenderingDeviceCreateInfo& createInfo);

		void CreateDescriptorSetLayout(const std::string& name, VkDescriptorType type, VkShaderStageFlags stageFlags);
		[[nodiscard]] RenderingApi::DescriptorSetLayout* GetDescriptorSetLayout(const std::string& name);

		void CreatePipelineLayout(const std::string& name, const std::function<void(PipelineLayoutBlueprint*)>& callback);
		[[nodiscard]] RenderingApi::PipelineLayout* GetPipelineLayout(const std::string& name);

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
		bool Internal_CreateDescriptorSet(std::string_view layoutType, RenderingApi::DescriptorSet* newDescriptorSet) const;

	public:
		[[nodiscard]] Handle CreateDescriptorSet(std::string_view layoutType);
		[[nodiscard]] Handle CreateUniformBufferDescriptorSet(const Handle& uniformBufferHandle, size_t size, std::string_view layoutType);
		[[nodiscard]] Handle CreateStorageBufferDescriptorSet(const Handle& uniformBufferHandle, size_t size, std::string_view layoutType);
		[[nodiscard]] Handle CreateTextureDescriptorSet(const Handle& textureHandle, const Handle& samplerHandle, std::string_view layoutType);
		void						 RequestDescriptorSetUpdate(const DescriptorSetUpdateRequest& request);
		void						 FlushDescriptorSetUpdateRequests();
		void						 DestroyDescriptorSet(const Handle& handle);
		RenderingApi::DescriptorSet* GetDescriptorSet(const Handle& handle);

		[[nodiscard]] Handle CreateShader(const ShaderDescription& desc);
		void				 DestroyShader(const Handle& handle);

		[[nodiscard]] Handle				  CreatePipeline(const PipelineDescription& desc);
		[[nodiscard]] RenderingApi::Pipeline* GetPipeline(const Handle& handle);
		void								  DestroyPipeline(const Handle& handle);

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
		[[nodiscard]] RenderingApi::SwapChain* GetSwapChain() const;

		[[nodiscard]] auto GetMaxFramesInFlight() const -> uint32_t;
		[[nodiscard]] auto GetAspectRatio() const -> float;

		[[nodiscard]] RenderingApi::DescriptorSetLayout* GetDescriptorSetLayout(const std::string& layoutType) const;

	private:
		RenderingDeviceCreateInfo				  m_createInfo;
		std::unique_ptr<RenderingDeviceInternals> m_self;
		uint32_t								  m_maxFramesInFlight;
		uint32_t								  m_currentFrameIndex = 0;
	};
};
