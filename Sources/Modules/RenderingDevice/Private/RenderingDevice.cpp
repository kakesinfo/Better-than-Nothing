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

#include "RenderingApi/Resources/Buffer.hpp"
#include "RenderingApi/Resources/DescriptorSet.hpp"
#include "RenderingApi/Resources/Sampler.hpp"
#include "RenderingApi/Resources/Texture.hpp"
#include "RenderingDevice/BufferDescription.hpp"
#include "RenderingDevice/DescriptorSetUpdateRequest.hpp"
#include "RenderingDevice/Handle.hpp"

#include <Module_Base/Logger.hpp>
#include <Module_Platform/Manager/WindowManager.hpp>
#include <Module_Platform/Window.hpp>
#include <RenderingApi.hpp>
#include <cmath>

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

		RegisterDescriptorLayout(DescriptorSetLayoutType::GlobalUniforms, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		RegisterDescriptorLayout(DescriptorSetLayoutType::Textures, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		RegisterDescriptorLayout(DescriptorSetLayoutType::PointLights, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
	}

	RenderingDevice::~RenderingDevice()
	{
		m_samplersLUT.clear();

		for (std::uint32_t i = 0; i < m_samplers.Size(); i++)
		{
			m_device->DestroySampler(m_samplers.Get(i)->value);
			m_samplers.Destroy(i);
		}

		for (std::uint32_t i = 0; i < m_buffers.Size(); i++)
		{
			const auto buffer = m_buffers.Get(i);
			m_allocator->DestroyBuffer(buffer->value, buffer->allocation);
			m_buffers.Destroy(i);
		}

		for (std::uint32_t i = 0; i < m_textures.Size(); i++)
		{
			const auto texture = m_textures.Get(i);
			m_device->DestroyImageView(texture->defaultView);
			m_allocator->DestroyImage(texture->value, texture->allocation);
			m_textures.Destroy(i);
		}
	};

	void RenderingDevice::RegisterDescriptorLayout(const DescriptorSetLayoutType key, const VkDescriptorType type, const VkShaderStageFlags stageFlags)
	{
		m_layouts[key] = RenderingApi::DescriptorSetLayout::Create({m_device.get(), type, 1, stageFlags});
	}

	std::unique_ptr<RenderingDevice> RenderingDevice::Create(const RenderingDeviceCreateInfo& createInfo)
	{
		return std::make_unique<RenderingDevice>(createInfo);
	}

	static VkFilter Internal_RetrieveSamplerFilter(const SamplerFilter& filter)
	{
		switch (filter)
		{
			case SamplerFilter::Linear:
				return VK_FILTER_LINEAR;
			case SamplerFilter::Nearest:
				return VK_FILTER_NEAREST;
		}
		throw std::runtime_error("Unknown sampler filter");
	}

	static VkSamplerAddressMode Internal_RetrieveSamplerAddressMode(const SamplerAddressMode& addressMode)
	{
		switch (addressMode)
		{
			case SamplerAddressMode::ClampToBorder:
				return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			case SamplerAddressMode::ClampToEdge:
				return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case SamplerAddressMode::MirrorRepeat:
				return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			case SamplerAddressMode::Repeat:
				return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		}
		throw std::runtime_error("Unknown sampler adress mode");
	}

	static VkBorderColor Internal_RetrieveBolderColor(const BorderColor& borderColor)
	{
		switch (borderColor)
		{
			case BorderColor::FloatOpaqueBlack:
				return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			case BorderColor::FloatOpaqueWhite:
				return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			case BorderColor::FloatTransparentBlack:
				return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
			case BorderColor::IntOpaqueBlack:
				return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			case BorderColor::IntOpaqueWhite:
				return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
			case BorderColor::IntTransparentBlack:
				return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
		}
		throw std::runtime_error("Unknown border color");
	}

	static VkCompareOp Internal_RetrieveCompareOp(const CompareOp& compareOp)
	{
		switch (compareOp)
		{
			case CompareOp::Always:
				return VK_COMPARE_OP_ALWAYS;
			case CompareOp::Equal:
				return VK_COMPARE_OP_EQUAL;
			case CompareOp::Greater:
				return VK_COMPARE_OP_GREATER;
			case CompareOp::GreaterOrEqual:
				return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case CompareOp::Less:
				return VK_COMPARE_OP_LESS;
			case CompareOp::LessOrEqual:
				return VK_COMPARE_OP_LESS_OR_EQUAL;
			case CompareOp::Never:
				return VK_COMPARE_OP_NEVER;
			case CompareOp::NotEqual:
				return VK_COMPARE_OP_NOT_EQUAL;
		}
		throw std::runtime_error("Unknown compare op");
	}

	static VkSamplerMipmapMode Internal_RetrieveSamplerMipmapMode(const SamplerMipmapMode& mipmapMode)
	{
		switch (mipmapMode)
		{
			case SamplerMipmapMode::Linear:
				return VK_SAMPLER_MIPMAP_MODE_LINEAR;
			case SamplerMipmapMode::Nearest:
				return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		}
		throw std::runtime_error("Unknown sampler mipmap mode");
	}

	Handle RenderingDevice::GetOrCreateTextureSampler(const SamplerDescription& desc)
	{
		if (m_samplersLUT.contains(desc))
			return m_samplersLUT[desc];

		const Handle		   handle = m_samplers.Allocate();
		RenderingApi::Sampler* newSampler = m_samplers.Get(handle);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = Internal_RetrieveSamplerFilter(desc.magFilter);
		samplerInfo.minFilter = Internal_RetrieveSamplerFilter(desc.minFilter);
		samplerInfo.addressModeU = Internal_RetrieveSamplerAddressMode(desc.addressModeU);
		samplerInfo.addressModeV = Internal_RetrieveSamplerAddressMode(desc.addressModeV);
		samplerInfo.addressModeW = Internal_RetrieveSamplerAddressMode(desc.addressModeW);
		samplerInfo.anisotropyEnable = desc.anisotropyEnable;
		samplerInfo.maxAnisotropy = desc.maxAnisotropy == -1 ? m_device->GetMaxAnisotropy() : desc.maxAnisotropy;
		samplerInfo.borderColor = Internal_RetrieveBolderColor(desc.borderColor);
		samplerInfo.compareEnable = desc.compareEnable;
		samplerInfo.compareOp = Internal_RetrieveCompareOp(desc.compareOp);
		samplerInfo.mipmapMode = Internal_RetrieveSamplerMipmapMode(desc.mipmapMode);
		samplerInfo.minLod = desc.minLod;
		samplerInfo.maxLod = desc.maxLod;
		samplerInfo.mipLodBias = desc.mipLodBias;

		if (m_device->CreateSampler(samplerInfo, &newSampler->value) != VK_SUCCESS)
			throw std::runtime_error("failed to create vulkan sampler");

		m_samplersLUT[desc] = handle;

		return handle;
	}

	void RenderingDevice::DestroyTextureSampler(const Handle& handle)
	{
		if (!m_samplers.Has(handle))
			throw std::runtime_error("Invalid sampler handle!");

		const RenderingApi::Sampler* sampler = m_samplers.Get(handle);
		m_device->DestroySampler(sampler->value);
		m_samplers.Destroy(handle);
	}

	RenderingApi::Sampler* RenderingDevice::GetSampler(const Handle& handle)
	{
		if (!m_samplers.Has(handle))
			throw std::runtime_error("Invalid sampler handle!");

		return m_samplers.Get(handle);
	}

	static VmaMemoryUsage Internal_RetrieveMemoryUsage(const BufferMemoryUsage& memoryUsage)
	{
		switch (memoryUsage)
		{
			case BufferMemoryUsage::Auto:
				return VMA_MEMORY_USAGE_AUTO;
			case BufferMemoryUsage::CpuCopy:
				return VMA_MEMORY_USAGE_CPU_COPY;
			case BufferMemoryUsage::CpuOnly:
				return VMA_MEMORY_USAGE_CPU_ONLY;
			case BufferMemoryUsage::GpuOnly:
				return VMA_MEMORY_USAGE_GPU_ONLY;
			case BufferMemoryUsage::CpuToGpu:
				return VMA_MEMORY_USAGE_CPU_TO_GPU;
			case BufferMemoryUsage::GpuToCpu:
				return VMA_MEMORY_USAGE_GPU_TO_CPU;
			case BufferMemoryUsage::None:
			default:
				throw std::runtime_error("Invalid memory usage");
		}
	}

	static bool Internal_CreateBuffer(const BufferDescription& desc, RenderingApi::Buffer* buffer, const RenderingApi::Allocator* allocator)
	{
		VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		bufferInfo.size = desc.size;
		bufferInfo.usage = static_cast<VkBufferUsageFlags>(desc.bufferUsage);

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = Internal_RetrieveMemoryUsage(desc.memoryUsage);

		if (desc.allocationFlags & BufferAllocationFlags::Mapped)
			allocInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
		if (desc.allocationFlags & BufferAllocationFlags::Dedicated)
			allocInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

		if (allocator->CreateBuffer(&bufferInfo, &allocInfo, &buffer->value, &buffer->allocation, &buffer->allocInfo) != VK_SUCCESS)
			return false;

		buffer->size = desc.size;

		return true;
	}

	Handle RenderingDevice::CreateBuffer(const BufferDescription& desc)
	{
		const Handle handle = m_buffers.Allocate();

		if (!Internal_CreateBuffer(desc, m_buffers.Get(handle), m_allocator.get()))
		{
			m_buffers.Destroy(handle);
			throw std::runtime_error("Failed to vulkan create buffer!");
		}

		return handle;
	}

	Handle RenderingDevice::CreateVertexBuffer(const void* data, const size_t size)
	{
		BufferDescription desc{};
		desc.size = size;
		desc.bufferUsage = BufferUsageFlags::TransferDst | BufferUsageFlags::Vertex;
		desc.memoryUsage = BufferMemoryUsage::GpuOnly;

		const Handle handle = CreateBuffer(desc);

		if (!CopyDataToBuffer(handle, data, size))
			throw std::runtime_error("Failed to copy data to buffer!");

		return handle;
	}

	Handle RenderingDevice::CreateIndexBuffer(const void* data, const size_t size)
	{
		BufferDescription desc{};
		desc.size = size;
		desc.bufferUsage = BufferUsageFlags::TransferDst | BufferUsageFlags::Index;
		desc.memoryUsage = BufferMemoryUsage::GpuOnly;

		const Handle handle = CreateBuffer(desc);

		if (!CopyDataToBuffer(handle, data, size))
			throw std::runtime_error("Failed to copy data to buffer!");

		return handle;
	}

	bool RenderingDevice::CopyDataToBuffer(const Handle& dstBufferHandle, const void* data, const size_t size)
	{
		if (data == nullptr || size == 0)
			return false;

		VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		RenderingApi::Buffer stagingBuffer; // TODO: replace this with a persistent 128B shared buffer

		if (m_allocator->CreateBuffer(&bufferInfo, &allocInfo, &stagingBuffer.value, &stagingBuffer.allocation, &stagingBuffer.allocInfo) != VK_SUCCESS)
			return false;

		if (!stagingBuffer.allocInfo.pMappedData)
			return false;

		memcpy(stagingBuffer.allocInfo.pMappedData, data, size);

		RenderingApi::CommandBuffer::SingleTimeCommands(
			[&](const std::unique_ptr<RenderingApi::CommandBuffer>& cmd) {
				const VkBuffer dstBuffer = m_buffers.Get(dstBufferHandle)->value;
				cmd->CopyBuffer(stagingBuffer.value, dstBuffer, size);
			},
			m_device.get(), m_commandPool.get(), m_graphicsQueue.get()
		);

		m_allocator->DestroyBuffer(stagingBuffer.value, stagingBuffer.allocation);

		return true;
	}

	void RenderingDevice::DestroyBuffer(const Handle& handle)
	{
		if (!m_buffers.Has(handle))
			throw std::runtime_error("Invalid buffer handle!");

		const RenderingApi::Buffer* buffer = m_buffers.Get(handle);
		m_allocator->DestroyBuffer(buffer->value, buffer->allocation);
		m_buffers.Destroy(handle);
	}

	RenderingApi::Buffer* RenderingDevice::GetBuffer(const Handle& handle)
	{
		if (!m_buffers.Has(handle))
			throw std::runtime_error("Invalid buffer handle!");
		return m_buffers.Get(handle);
	}

	static std::uint8_t Internal_CalculateMipLevels(const std::uint32_t width, const std::uint32_t height)
	{
		return static_cast<std::uint8_t>(std::floor(std::log2(std::max(width, height)))) + 1;
	}

	bool RenderingDevice::Internal_CreateTextureImage(const TextureImageDescription& desc)
	{
		RenderingApi::Texture* texture = m_textures.Get(desc.destination);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent = {desc.width, desc.height, desc.depth};
		imageInfo.mipLevels = desc.mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = desc.format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = desc.initialLayout;
		imageInfo.usage = desc.usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = desc.samples;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		allocCreateInfo.flags = desc.dedicated ? VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT : 0;
		allocCreateInfo.priority = 1.0f;

		if (m_allocator->CreateImage(&imageInfo, &allocCreateInfo, &texture->value, &texture->allocation, nullptr) != VK_SUCCESS)
			return false;

		texture->width = desc.width;
		texture->height = desc.height;
		texture->mipLevels = desc.mipLevels;
		texture->format = desc.format;

		return true;
	}

	bool RenderingDevice::Internal_CreateTextureImageView(const TextureImageViewDescription& desc)
	{
		RenderingApi::Texture* texture = m_textures.Get(desc.destination);

		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = texture->value;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = texture->format;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = desc.aspectFlags;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = texture->mipLevels;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (m_device->CreateImageView(createInfo, &texture->defaultView) != VK_SUCCESS)
			return false;

		return true;
	}

	bool RenderingDevice::Internal_CopyDataToTextureImage(const CopyDataToTextureImageDescription& desc)
	{
		const RenderingApi::Texture* texture = m_textures.Get(desc.destination);

		VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		bufferInfo.size = desc.dataSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		RenderingApi::Buffer stagingBuffer; // TODO: replace this with a persistent 64MB shared buffer
		if (m_allocator->CreateBuffer(&bufferInfo, &allocInfo, &stagingBuffer.value, &stagingBuffer.allocation, &stagingBuffer.allocInfo) != VK_SUCCESS)
			return false;

		if (!stagingBuffer.allocInfo.pMappedData)
			return false;

		memcpy(stagingBuffer.allocInfo.pMappedData, desc.data, desc.dataSize);

		RenderingApi::CommandBuffer::SingleTimeCommands(
			[&](const std::unique_ptr<RenderingApi::CommandBuffer>& commandBuffer) {
				commandBuffer->TransitionImageLayout(texture->value, desc.format, desc.oldLayout, desc.newLayout, texture->mipLevels);
				commandBuffer->CopyBufferToImage(stagingBuffer.value, texture->value, texture->width, texture->height);

				// Transition to TRANSFER_DST_OPTIMAL is done in mipmap generation _createMipmaps
				// commandBuffer->TransitionImageLayout(vkImage, VK_FORMAT_R8G8B8A8_SRGB,
				// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);
			},
			m_device.get(), m_commandPool.get(), m_graphicsQueue.get()
		);

		m_allocator->DestroyBuffer(stagingBuffer.value, stagingBuffer.allocation);

		return true;
	}

	bool RenderingDevice::Internal_CreateTextureMipmaps(const Handle& destination)
	{
		const RenderingApi::Texture* texture = m_textures.Get(destination);

		if (!m_device->IsSupportedFormatFeature(texture->format, VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
			return false;

		RenderingApi::CommandBuffer::SingleTimeCommands(
			[&](const std::unique_ptr<RenderingApi::CommandBuffer>& commandBuffer) {
				VkImageMemoryBarrier barrier{};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.image = texture->value;
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Must not create mipmaps for depth images
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount = 1;
				barrier.subresourceRange.levelCount = 1;

				auto mipWidth = static_cast<std::int32_t>(texture->width);
				auto mipHeight = static_cast<std::int32_t>(texture->height);
				for (std::uint8_t i = 1; i < texture->mipLevels; i++)
				{
					barrier.subresourceRange.baseMipLevel = i - 1;
					barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

					commandBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

					VkImageBlit blit{};
					blit.srcOffsets[0] = {0, 0, 0};
					blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
					blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					blit.srcSubresource.mipLevel = i - 1;
					blit.srcSubresource.baseArrayLayer = 0;
					blit.srcSubresource.layerCount = 1;
					blit.dstOffsets[0] = {0, 0, 0};
					blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
					blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					blit.dstSubresource.mipLevel = i;
					blit.dstSubresource.baseArrayLayer = 0;
					blit.dstSubresource.layerCount = 1;

					commandBuffer->BlitImage(texture->value, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture->value, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &blit, VK_FILTER_LINEAR);

					barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

					commandBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

					if (mipWidth > 1)
						mipWidth /= 2;
					if (mipHeight > 1)
						mipHeight /= 2;
				}

				barrier.subresourceRange.baseMipLevel = texture->mipLevels - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				commandBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
			},
			m_device.get(), m_commandPool.get(), m_graphicsQueue.get()
		);

		return true;
	}

	Handle RenderingDevice::CreateTextureFromData(const TextureFromDataDescription& desc)
	{
		const Handle	   handle = m_textures.Allocate();
		const std::uint8_t mipLevels = Internal_CalculateMipLevels(desc.width, desc.height);

		TextureImageDescription imageDesc{};
		imageDesc.destination = handle;
		imageDesc.width = desc.width;
		imageDesc.height = desc.height;
		imageDesc.depth = 1;
		imageDesc.mipLevels = mipLevels;
		imageDesc.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageDesc.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

		if (!Internal_CreateTextureImage(imageDesc))
			throw std::runtime_error("Cannot create texture image from data");

		CopyDataToTextureImageDescription copyDesc;
		copyDesc.destination = handle;
		copyDesc.data = desc.data;
		copyDesc.dataSize = desc.dataSize;
		copyDesc.format = imageDesc.format;
		copyDesc.oldLayout = imageDesc.initialLayout;

		if (!Internal_CopyDataToTextureImage(copyDesc))
			throw std::runtime_error("Cannot copy data to texture image using staging buffer");

		if (!Internal_CreateTextureMipmaps(handle))
			throw std::runtime_error("Cannot create texture mip maps");

		TextureImageViewDescription viewDesc{};
		viewDesc.destination = handle;
		viewDesc.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

		if (!Internal_CreateTextureImageView(viewDesc))
			throw std::runtime_error("Cannot create texture image view");

		return handle;
	}

	void RenderingDevice::DestroyTexture(const Handle& handle)
	{
		if (!m_textures.Has(handle))
			throw std::runtime_error("Invalid texture handle!");

		const RenderingApi::Texture* texture = m_textures.Get(handle);
		m_device->DestroyImageView(texture->defaultView);
		m_allocator->DestroyImage(texture->value, texture->allocation);
		m_textures.Destroy(handle);
	}

	RenderingApi::Texture* RenderingDevice::GetTexture(const Handle& handle)
	{
		if (!m_textures.Has(handle))
			throw std::runtime_error("Invalid texture handle!");
		return m_textures.Get(handle);
	}

	bool RenderingDevice::Internal_CreateDescriptorSet(const DescriptorSetLayoutType& type, RenderingApi::DescriptorSet* newDescriptorSet) const
	{
		const auto layout = GetDescriptorSetLayout(type)->Handle();

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPool->Handle();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		return vkAllocateDescriptorSets(m_device->Handle(), &allocInfo, &newDescriptorSet->value) == VK_SUCCESS;
	}

	Handle RenderingDevice::CreateDescriptorSet(DescriptorSetLayoutType layoutType)
	{
		const Handle handle = m_descriptors.Allocate();

		if (!Internal_CreateDescriptorSet(layoutType, m_descriptors.Get(handle)))
		{
			m_descriptors.Destroy(handle);
			throw std::runtime_error("Failed to allocate descriptor set");
		}

		return handle;
	}

	Handle RenderingDevice::CreateTextureDescriptorSet(const Handle& textureHandle, const Handle& samplerHandle)
	{
		const Handle handle = CreateDescriptorSet(DescriptorSetLayoutType::Textures);

		const RenderingApi::DescriptorSet* descriptor = GetDescriptorSet(handle);
		const RenderingApi::Texture*	   texture = GetTexture(textureHandle);
		const RenderingApi::Sampler*	   sampler = GetSampler(samplerHandle);

		DescriptorSetUpdateRequest request{};

		request.imageInfo = {};
		request.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		request.imageInfo.imageView = texture->defaultView;
		request.imageInfo.sampler = sampler->value;

		request.write = {};
		request.write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		request.write.dstSet = descriptor->value;
		request.write.dstBinding = 0;
		request.write.dstArrayElement = 0;
		request.write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		request.write.descriptorCount = 1;
		request.write.pImageInfo = &request.imageInfo;

		RequestDescriptorSetUpdate(request);

		return handle;
	}

	void RenderingDevice::RequestDescriptorSetUpdate(const DescriptorSetUpdateRequest& request)
	{
		m_descriptorsUpdateRequests.push_back(request);
	}

	void RenderingDevice::FlushDescriptorSetUpdateRequests()
	{
		if (m_descriptorsUpdateRequests.empty())
			return;

		m_descriptorsWrites.clear();
		m_descriptorsWrites.reserve(m_descriptorsUpdateRequests.size());

		for (const auto& request : m_descriptorsUpdateRequests)
		{
			VkWriteDescriptorSet write = request.write;

			if (write.descriptorCount > 1)
				throw std::runtime_error("Cannot update more than one descriptor set at once");

			switch (write.descriptorType)
			{
				case VK_DESCRIPTOR_TYPE_SAMPLER:
				case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
				case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
				case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
				case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
					write.pImageInfo = &request.imageInfo;
					break;

				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
				case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
					write.pBufferInfo = &request.bufferInfo;
					break;

				default:
					throw std::runtime_error("Invalid descriptor type!");
			}

			m_descriptorsWrites.push_back(write);
		}

		if (!m_descriptorsWrites.empty())
		{
			const auto					writesCount = static_cast<std::uint32_t>(m_descriptorsWrites.size());
			const VkWriteDescriptorSet* writesData = m_descriptorsWrites.data();

			vkUpdateDescriptorSets(m_device->Handle(), writesCount, writesData, 0, nullptr);
		}

		m_descriptorsUpdateRequests.clear();
	}

	void RenderingDevice::DestroyDescriptorSet(const Handle& handle)
	{
		if (!m_descriptors.Has(handle))
			throw std::runtime_error("Invalid descriptor set handle!");

		m_descriptors.Destroy(handle);
	}

	RenderingApi::DescriptorSet* RenderingDevice::GetDescriptorSet(const Handle& handle)
	{
		if (!m_descriptors.Has(handle))
			throw std::runtime_error("Invalid descriptor set handle!");

		return m_descriptors.Get(handle);
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

	RenderingApi::DescriptorSetLayout* RenderingDevice::GetDescriptorSetLayout(DescriptorSetLayoutType key) const
	{
		if (!m_layouts.contains(key))
			throw std::runtime_error("Cannot find descriptorSetLayout");
		return m_layouts.at(key).get();
	}
}
