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

#include "RenderingApi/Resources/Buffer.hpp"
#include "RenderingApi/Resources/Texture.hpp"
#include "RenderingDevice/Handle.hpp"
#include "RenderingDevice/RenderingDevice.hpp"
#include "RenderingDevice_FormatUtils.hpp"
#include "RenderingDevice_Internals.hpp"
#include "StagingBuffer.hpp"
#include <RenderingApi.hpp>
#include <cmath>

namespace BetterThanNothing
{
	static std::uint8_t Internal_CalculateMipLevels(const std::uint32_t width, const std::uint32_t height)
	{
		return static_cast<std::uint8_t>(std::floor(std::log2(std::max(width, height)))) + 1;
	}

	VkFormat ToVkFormat(const TextureFormat format, const VkFormat fallback)
	{
		switch (format)
		{
			case TextureFormat::R8G8B8A8_UNORM:
				return VK_FORMAT_R8G8B8A8_UNORM;
			case TextureFormat::R8G8B8A8_SRGB:
				return VK_FORMAT_R8G8B8A8_SRGB;
			case TextureFormat::B8G8R8A8_UNORM:
				return VK_FORMAT_B8G8R8A8_UNORM;
			case TextureFormat::R16_SFLOAT:
				return VK_FORMAT_R16_SFLOAT;
			case TextureFormat::R16G16_SFLOAT:
				return VK_FORMAT_R16G16_SFLOAT;
			case TextureFormat::R16G16B16A16_SFLOAT:
				return VK_FORMAT_R16G16B16A16_SFLOAT;
			case TextureFormat::R32_SFLOAT:
				return VK_FORMAT_R32_SFLOAT;
			case TextureFormat::D32_SFLOAT:
				return VK_FORMAT_D32_SFLOAT;
			case TextureFormat::D24_UNORM_S8_UINT:
				return VK_FORMAT_D24_UNORM_S8_UINT;
			case TextureFormat::D32_SFLOAT_S8_UINT:
				return VK_FORMAT_D32_SFLOAT_S8_UINT;
			case TextureFormat::Undefined:
			default:
				return fallback;
		}
	}

	static VkFormat Internal_ToVkFormat(const TextureFormat format, const RenderingDeviceInternals* self)
	{
		return ToVkFormat(format, self->swapChain->GetImageFormat());
	}

	static VkImageUsageFlags Internal_ToVkTextureUsage(const TextureUsageFlags usage)
	{
		VkImageUsageFlags flags = 0;
		if (usage & TextureUsageFlags::TransferSrc)
			flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (usage & TextureUsageFlags::TransferDst)
			flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		if (usage & TextureUsageFlags::Sampled)
			flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
		if (usage & TextureUsageFlags::Storage)
			flags |= VK_IMAGE_USAGE_STORAGE_BIT;
		if (usage & TextureUsageFlags::ColorAttachment)
			flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (usage & TextureUsageFlags::DepthStencilAttachment)
			flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		if (usage & TextureUsageFlags::TransientAttachment)
			flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		return flags;
	}

	static VkImageAspectFlags Internal_AspectFlagsForFormat(const VkFormat format)
	{
		switch (format)
		{
			case VK_FORMAT_D32_SFLOAT:
				return VK_IMAGE_ASPECT_DEPTH_BIT;
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
			case VK_FORMAT_D24_UNORM_S8_UINT:
				return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			default:
				return VK_IMAGE_ASPECT_COLOR_BIT;
		}
	}

	bool RenderingDevice::Internal_CreateTextureImage(const TextureImageDescription& desc)
	{
		RenderingApi::Texture* texture = m_self->textures.Get(desc.destination);

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

		if (m_self->allocator->CreateImage(&imageInfo, &allocCreateInfo, &texture->value, &texture->allocation, nullptr) != VK_SUCCESS)
			return false;

		texture->width = desc.width;
		texture->height = desc.height;
		texture->mipLevels = desc.mipLevels;
		texture->format = desc.format;
		texture->samples = desc.samples;
		texture->aspectMask = Internal_AspectFlagsForFormat(desc.format);
		texture->currentLayout = desc.initialLayout;

		return true;
	}

	bool RenderingDevice::Internal_CreateTextureImageView(const TextureImageViewDescription& desc)
	{
		RenderingApi::Texture* texture = m_self->textures.Get(desc.destination);

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

		if (m_self->device->CreateImageView(createInfo, &texture->defaultView) != VK_SUCCESS)
			return false;

		return true;
	}

	void RenderingDevice::Internal_UploadTextureAndGenerateMipmaps(const CopyDataToTextureImageDescription& desc, const bool generateMipmaps)
	{
		RenderingApi::Texture* texture = m_self->textures.Get(desc.destination);

		Internal_EnsureUploadBuffer(desc.dataSize, 4);

		const uint64_t				 offset = m_self->stagingBuffer->Stage(desc.data, desc.dataSize, 4);
		RenderingApi::CommandBuffer* commandBuffer = m_self->uploadCommandBuffer.get();

		commandBuffer->TransitionImageLayout2(texture->value, texture->aspectMask, texture->mipLevels, desc.oldLayout, desc.newLayout);
		commandBuffer->CopyBufferToImage(m_self->stagingBuffer->GetBuffer(), texture->value, texture->width, texture->height, offset);

		if (!generateMipmaps || texture->mipLevels <= 1)
		{
			commandBuffer->TransitionImageLayout2(texture->value, texture->aspectMask, texture->mipLevels, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		else
		{
			VkImageMemoryBarrier2 barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			barrier.image = texture->value;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

			VkDependencyInfo depInfo{};
			depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
			depInfo.imageMemoryBarrierCount = 1;
			depInfo.pImageMemoryBarriers = &barrier;

			auto mipWidth = static_cast<std::int32_t>(texture->width);
			auto mipHeight = static_cast<std::int32_t>(texture->height);
			for (std::uint8_t i = 1; i < texture->mipLevels; i++)
			{
				barrier.subresourceRange.baseMipLevel = i - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
				barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
				barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
				barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;

				commandBuffer->PipelineBarrier2(depInfo);

				VkImageBlit blit{};
				blit.srcOffsets[0] = {0, 0, 0};
				blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
				blit.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, static_cast<uint32_t>(i - 1), 0, 1};
				blit.dstOffsets[0] = {0, 0, 0};
				blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
				blit.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, static_cast<uint32_t>(i), 0, 1};

				commandBuffer->BlitImage(texture->value, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture->value, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &blit, VK_FILTER_LINEAR);

				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
				barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
				barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
				barrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;

				commandBuffer->PipelineBarrier2(depInfo);

				if (mipWidth > 1)
					mipWidth /= 2;
				if (mipHeight > 1)
					mipHeight /= 2;
			}

			barrier.subresourceRange.baseMipLevel = texture->mipLevels - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
			barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
			barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
			barrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;

			commandBuffer->PipelineBarrier2(depInfo);
		}

		texture->currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		if (m_self->uploadBatchDepth == 0)
			Internal_FlushUploadBuffer();
	}

	Handle RenderingDevice::CreateTexture(const TextureCreateInfo& desc)
	{
		if (desc.width == 0 || desc.height == 0)
			throw std::runtime_error("Invalid texture dimensions");

		const VkFormat			   vkFormat = Internal_ToVkFormat(desc.format, m_self.get());
		const VkSampleCountFlagBits samples = desc.useMsaa ? m_self->device->GetMaxMsaaSamples() : VK_SAMPLE_COUNT_1_BIT;
		const Handle			   handle = m_self->textures.Allocate();

		try
		{
			TextureImageDescription imageDesc{};
			imageDesc.destination = handle;
			imageDesc.width = desc.width;
			imageDesc.height = desc.height;
			imageDesc.depth = 1;
			imageDesc.mipLevels = desc.mipLevels > 0 ? desc.mipLevels : 1;
			imageDesc.format = vkFormat;
			imageDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageDesc.usage = Internal_ToVkTextureUsage(desc.usage);
			imageDesc.samples = samples;
			imageDesc.dedicated = desc.dedicated;

			if (!Internal_CreateTextureImage(imageDesc))
				throw std::runtime_error("Cannot create texture image");

			TextureImageViewDescription viewDesc{};
			viewDesc.destination = handle;
			viewDesc.aspectFlags = Internal_AspectFlagsForFormat(vkFormat);

			if (!Internal_CreateTextureImageView(viewDesc))
				throw std::runtime_error("Cannot create texture image view");
		}
		catch (...)
		{
			DestroyTexture(handle);
			throw;
		}

		return handle;
	}

	Handle RenderingDevice::CreateColorAttachment(const uint32_t width, const uint32_t height, const bool useMsaa)
	{
		TextureCreateInfo desc{};
		desc.width = width;
		desc.height = height;
		desc.format = TextureFormat::Undefined; // Uses swapchain format
		desc.usage = TextureUsageFlags::ColorAttachment | (useMsaa ? TextureUsageFlags::TransientAttachment : TextureUsageFlags::Sampled);
		desc.useMsaa = useMsaa;
		desc.dedicated = true;
		desc.mipLevels = 1;
		return CreateTexture(desc);
	}

	Handle RenderingDevice::CreateDepthAttachment(const uint32_t width, const uint32_t height, const bool useMsaa)
	{
		const VkFormat depthFormat = m_self->device->FindDepthFormat();
		TextureFormat  engineFormat = TextureFormat::D32_SFLOAT;
		if (depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT)
			engineFormat = TextureFormat::D32_SFLOAT_S8_UINT;
		else if (depthFormat == VK_FORMAT_D24_UNORM_S8_UINT)
			engineFormat = TextureFormat::D24_UNORM_S8_UINT;

		TextureCreateInfo desc{};
		desc.width = width;
		desc.height = height;
		desc.format = engineFormat;
		desc.usage = TextureUsageFlags::DepthStencilAttachment;
		desc.useMsaa = useMsaa;
		desc.dedicated = true;
		desc.mipLevels = 1;
		return CreateTexture(desc);
	}

	Handle RenderingDevice::CreateTextureFromData(const TextureFromDataDescription& desc)
	{
		if (desc.data == nullptr || desc.dataSize == 0 || desc.width == 0 || desc.height == 0)
			throw std::runtime_error("Invalid texture data description");

		const VkFormat	   format = Internal_ToVkFormat(desc.format, m_self.get());
		const bool		   canGenerateMipmaps = desc.generateMipmaps && m_self->device->IsSupportedFormatFeature(format, VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT);
		const std::uint8_t mipLevels = canGenerateMipmaps ? Internal_CalculateMipLevels(desc.width, desc.height) : 1;

		const Handle handle = m_self->textures.Allocate();

		try
		{
			TextureImageDescription imageDesc{};
			imageDesc.destination = handle;
			imageDesc.width = desc.width;
			imageDesc.height = desc.height;
			imageDesc.depth = 1;
			imageDesc.mipLevels = mipLevels;
			imageDesc.format = format;
			imageDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageDesc.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | (canGenerateMipmaps ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0u);

			if (!Internal_CreateTextureImage(imageDesc))
				throw std::runtime_error("Cannot create texture image from data");

			CopyDataToTextureImageDescription copyDesc{};
			copyDesc.destination = handle;
			copyDesc.data = desc.data;
			copyDesc.dataSize = static_cast<uint32_t>(desc.dataSize);
			copyDesc.format = imageDesc.format;
			copyDesc.oldLayout = imageDesc.initialLayout;

			Internal_UploadTextureAndGenerateMipmaps(copyDesc, canGenerateMipmaps);

			TextureImageViewDescription viewDesc{};
			viewDesc.destination = handle;
			viewDesc.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

			if (!Internal_CreateTextureImageView(viewDesc))
				throw std::runtime_error("Cannot create texture image view");
		}
		catch (...)
		{
			DestroyTexture(handle);
			throw;
		}

		return handle;
	}

	void RenderingDevice::DestroyTexture(const Handle& handle)
	{
		if (!m_self->textures.Has(handle))
			throw std::runtime_error("Invalid texture handle!");

		const RenderingApi::Texture* texture = m_self->textures.Get(handle);
		const VkImageView			 view = texture->defaultView;
		const VkImage				 image = texture->value;
		const VmaAllocation			 allocation = texture->allocation;
		m_self->textures.Destroy(handle);

		RenderingApi::Device*	 device = m_self->device.get();
		RenderingApi::Allocator* allocator = m_self->allocator.get();
		Internal_EnqueueDeletion([device, allocator, view, image, allocation]() {
			device->DestroyImageView(view);
			allocator->DestroyImage(image, allocation);
		});
	}

	RenderingApi::Texture* RenderingDevice::GetTexture(const Handle& handle)
	{
		if (!m_self->textures.Has(handle))
			throw std::runtime_error("Invalid texture handle!");
		return m_self->textures.Get(handle);
	}
}
