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
#include "RenderingDevice_Internals.hpp"

#include "RenderingApi/Resources/Buffer.hpp"
#include "RenderingApi/Resources/Texture.hpp"
#include "RenderingDevice/Handle.hpp"
#include "StagingBuffer.hpp"

#include <Platform/LegacyWindowManager.hpp>
#include <RenderingApi.hpp>
#include <cmath>

namespace BetterThanNothing
{
	static std::uint8_t Internal_CalculateMipLevels(const std::uint32_t width, const std::uint32_t height)
	{
		return static_cast<std::uint8_t>(std::floor(std::log2(std::max(width, height)))) + 1;
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

	void RenderingDevice::Internal_CopyDataToTextureImage(const CopyDataToTextureImageDescription& desc)
	{
		const RenderingApi::Texture* texture = m_self->textures.Get(desc.destination);

		VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		bufferInfo.size = desc.dataSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		const uint64_t offset = m_self->stagingBuffer->Stage(desc.data, desc.dataSize);

		RenderingApi::CommandBuffer::SingleTimeCommands(
			[&](const std::unique_ptr<RenderingApi::CommandBuffer>& commandBuffer) {
				commandBuffer->TransitionImageLayout(texture->value, desc.format, desc.oldLayout, desc.newLayout, texture->mipLevels);
				commandBuffer->CopyBufferToImage(m_self->stagingBuffer->GetBuffer(), texture->value, texture->width, texture->height, offset);

				// Transition to TRANSFER_DST_OPTIMAL is done in mipmap generation _createMipmaps
				// commandBuffer->TransitionImageLayout(vkImage, VK_FORMAT_R8G8B8A8_SRGB,
				// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);
			},
			m_self->device.get(), m_self->commandPool.get(), m_self->graphicsQueue.get()
		);

		m_self->stagingBuffer->Reset();
	}

	bool RenderingDevice::Internal_CreateTextureMipmaps(const Handle& destination)
	{
		const RenderingApi::Texture* texture = m_self->textures.Get(destination);

		if (!m_self->device->IsSupportedFormatFeature(texture->format, VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
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
			m_self->device.get(), m_self->commandPool.get(), m_self->graphicsQueue.get()
		);

		return true;
	}

	Handle RenderingDevice::CreateTextureFromData(const TextureFromDataDescription& desc)
	{
		const Handle	   handle = m_self->textures.Allocate();
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

		Internal_CopyDataToTextureImage(copyDesc);

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
		if (!m_self->textures.Has(handle))
			throw std::runtime_error("Invalid texture handle!");

		const RenderingApi::Texture* texture = m_self->textures.Get(handle);
		m_self->device->DestroyImageView(texture->defaultView);
		m_self->allocator->DestroyImage(texture->value, texture->allocation);
		m_self->textures.Destroy(handle);
	}

	RenderingApi::Texture* RenderingDevice::GetTexture(const Handle& handle)
	{
		if (!m_self->textures.Has(handle))
			throw std::runtime_error("Invalid texture handle!");
		return m_self->textures.Get(handle);
	}
}
