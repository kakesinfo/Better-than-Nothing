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
#include "RenderPassDescription.hpp"
#include "SamplerDescription.hpp"
#include "TextureDescription.hpp"
#include "Types.hpp"

#include <Common/EBufferingMode.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace BetterThanNothing
{
	struct ShaderDescription;
	struct RenderingDeviceInternals;
	class DrawStreamView;

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
	struct TextureCreateInfo;
	struct PipelineDescription;
	class PipelineLayoutBlueprint;

	struct RenderingDeviceCreateInfo
	{
		Platform*	   platform;
		EBufferingMode bufferingMode = EBufferingMode::Double;
	};

	struct TextureBindingUpdate
	{
		uint32_t binding = 0;
		Handle	 textureHandle{};
		Handle	 samplerHandle{};
	};

	struct TlasInstanceDesc
	{
		float	 transform[12] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0}; // 3x4 row-major matrix
		Handle	 blasHandle{};
		uint32_t instanceCustomIndex = 0;
		uint32_t mask = 0xFF;
	};

	class RenderingDevice
	{
	public:
		explicit RenderingDevice(const RenderingDeviceCreateInfo& createInfo);
		~RenderingDevice();

		RenderingDevice(const RenderingDevice&) = delete;
		RenderingDevice& operator=(const RenderingDevice&) = delete;
		RenderingDevice(RenderingDevice&&) = delete;
		RenderingDevice& operator=(RenderingDevice&&) = delete;

		[[nodiscard]] static std::unique_ptr<RenderingDevice> Create(const RenderingDeviceCreateInfo& createInfo);

		// Layouts
		void CreateDescriptorSetLayout(const std::string& name, DescriptorType type, ShaderStageFlags stageFlags);
		void CreateDescriptorSetLayout(const std::string& name, std::span<const DescriptorBindingDesc> bindings);
		[[nodiscard]] RenderingApi::DescriptorSetLayout* GetDescriptorSetLayout(const std::string& name) const;

		void CreatePipelineLayout(const std::string& name, const std::function<void(PipelineLayoutBlueprint*)>& callback);
		[[nodiscard]] RenderingApi::PipelineLayout* GetPipelineLayout(const std::string& name) const;

		// Upload batching (batches multiple buffer/texture uploads into minimal GPU submissions)
		void BeginUploadBatch();
		void EndUploadBatch();

		class UploadBatchScope
		{
		public:
			explicit UploadBatchScope(RenderingDevice* device) : m_device(device)
			{
				if (m_device)
					m_device->BeginUploadBatch();
			}
			~UploadBatchScope()
			{
				if (m_device)
					m_device->EndUploadBatch();
			}
			UploadBatchScope(const UploadBatchScope&) = delete;
			UploadBatchScope& operator=(const UploadBatchScope&) = delete;

		private:
			RenderingDevice* m_device = nullptr;
		};

		// Samplers
		Handle				   GetOrCreateTextureSampler(const SamplerDescription& desc);
		void				   DestroyTextureSampler(const Handle& handle);
		RenderingApi::Sampler* GetSampler(const Handle& handle);

		// Buffers
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
		[[nodiscard]] uint64_t GetBufferDeviceAddress(const Handle& handle);

		// Acceleration Structures (Ray Query)
		[[nodiscard]] bool	 IsRayQuerySupported() const;
		[[nodiscard]] Handle CreateBlas(const Handle& vertexBuffer, uint32_t vertexCount, uint32_t vertexStride, const Handle& indexBuffer, uint32_t indexCount);
		void				 BuildOrUpdateTlas(Handle& tlasHandle, Handle& scratchBufferHandle, Handle& instanceBufferHandle, std::span<const TlasInstanceDesc> instances);
		void				 DestroyAccelerationStructure(const Handle& handle);

	private:
		bool Internal_CreateTextureImage(const TextureImageDescription& desc);
		bool Internal_CreateTextureImageView(const TextureImageViewDescription& desc);
		void Internal_UploadTextureAndGenerateMipmaps(const CopyDataToTextureImageDescription& desc, bool generateMipmaps);
		void Internal_EnsureUploadBuffer(uint64_t requiredStagingBytes, uint64_t alignment = 4);
		void Internal_FlushUploadBuffer();
		void Internal_EnqueueDeletion(std::function<void()>&& deletor);

	public:
		// Textures & Render Targets
		Handle				   CreateTexture(const TextureCreateInfo& desc);
		Handle				   CreateTextureFromData(const TextureFromDataDescription& desc);
		Handle				   CreateColorAttachment(uint32_t width, uint32_t height, bool useMsaa = true);
		Handle				   CreateDepthAttachment(uint32_t width, uint32_t height, bool useMsaa = true);
		void				   DestroyTexture(const Handle& handle);
		RenderingApi::Texture* GetTexture(const Handle& handle);

	private:
		bool Internal_CreateDescriptorSet(std::string_view layoutType, RenderingApi::DescriptorSet* newDescriptorSet) const;

	public:
		// Descriptor Sets
		[[nodiscard]] Handle		 CreateDescriptorSet(std::string_view layoutType);
		[[nodiscard]] Handle		 CreateUniformBufferDescriptorSet(const Handle& uniformBufferHandle, size_t size, std::string_view layoutType);
		[[nodiscard]] Handle		 CreateStorageBufferDescriptorSet(const Handle& storageBufferHandle, size_t size, std::string_view layoutType);
		[[nodiscard]] Handle		 CreateTextureDescriptorSet(const Handle& textureHandle, const Handle& samplerHandle, std::string_view layoutType);
		[[nodiscard]] Handle		 CreateCombinedImageDescriptorSet(std::span<const TextureBindingUpdate> bindings, std::string_view layoutType);
		void						 UpdateDescriptorBuffer(const Handle& setHandle, uint32_t binding, const Handle& bufferHandle, size_t size, DescriptorType type);
		void						 UpdateDescriptorImage(const Handle& setHandle, uint32_t binding, const Handle& textureHandle, const Handle& samplerHandle);
		void						 UpdateDescriptorStorageImage(const Handle& setHandle, uint32_t binding, const Handle& textureHandle);
		void						 UpdateDescriptorAccelerationStructure(const Handle& setHandle, uint32_t binding, const Handle& asHandle);
		void						 RequestDescriptorSetUpdate(const DescriptorSetUpdateRequest& request);
		void						 FlushDescriptorSetUpdateRequests();
		void						 DestroyDescriptorSet(const Handle& handle);
		RenderingApi::DescriptorSet* GetDescriptorSet(const Handle& handle);

		// Shaders & Pipelines
		[[nodiscard]] Handle				  CreateShader(const ShaderDescription& desc);
		void								  DestroyShader(const Handle& handle);
		[[nodiscard]] Handle				  CreatePipeline(const PipelineDescription& desc);
		[[nodiscard]] RenderingApi::Pipeline* GetPipeline(const Handle& handle);
		void								  DestroyPipeline(const Handle& handle);

	private:
		void Internal_CreateCommandBuffers();
		void Internal_CreateSyncObjects();

	public:
		// Frame lifecycle & high-level command recording (Vulkan-free API for Renderer)
		std::optional<uint32_t> BeginFrame();
		void					EndFrame(uint32_t imageIndex);

		void TransitionTexture(const Handle& textureHandle, ImageLayout newLayout);
		void TransitionSwapChainImage(uint32_t imageIndex, ImageLayout newLayout);
		void CopyTexture(const Handle& srcTextureHandle, const Handle& dstTextureHandle);
		void BeginRenderPass(const RenderPassDescriptor& desc);
		void EndRenderPass();
		void ExecuteDrawStream(const DrawStreamView& drawStream);
		void DrawFullscreen(const Handle& pipelineHandle, const std::string& layoutName, std::span<const Handle> descriptorSets, const void* pushConstantData = nullptr, uint32_t pushConstantSize = 0, ShaderStageFlags pushConstantStages = ShaderStageFlags::Fragment);

		void BindComputePipeline(const Handle& pipelineHandle);
		void BindComputeDescriptorSets(const std::string& layoutName, uint32_t firstSet, std::span<const Handle> descriptorSets);
		void PushComputeConstants(const std::string& layoutName, const void* data, uint32_t size);
		void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
		void ComputeBarrier();

		void WaitIdle() const;

		// Swapchain & device queries (engine-agnostic)
		[[nodiscard]] uint32_t					  GetCurrentFrameIndex() const;
		[[nodiscard]] uint32_t					  GetMaxFramesInFlight() const;
		[[nodiscard]] float						  GetAspectRatio() const;
		[[nodiscard]] std::pair<uint32_t, uint32_t> GetSwapChainExtent() const;
		[[nodiscard]] uint64_t					  GetSwapChainGeneration() const;
		[[nodiscard]] bool						  IsMsaaEnabled() const;

		// Backend interop (used by Editor ImGui Vulkan backend)
		[[nodiscard]] RenderingApi::CommandBuffer* GetCurrentCommandBuffer() const;
		[[nodiscard]] RenderingApi::Instance*	   GetInstance() const;
		[[nodiscard]] RenderingApi::Device*		   GetDevice() const;
		[[nodiscard]] RenderingApi::Queue*		   GetGraphicsQueue() const;
		[[nodiscard]] RenderingApi::SwapChain*	   GetSwapChain() const;

	private:
		RenderingDeviceCreateInfo				  m_createInfo;
		std::unique_ptr<RenderingDeviceInternals> m_self;
		uint32_t								  m_maxFramesInFlight;
		uint32_t								  m_currentFrameIndex = 0;
	};
};
