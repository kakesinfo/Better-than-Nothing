#include "ResourceManager/Loader/TextureLoader.hpp"
#include "ResourceManager/TextureGpuData.hpp"
#include <RenderingApi.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace BetterThanNothing
{
	TextureLoader::TextureLoader(VulkanContext* context) : ResourceLoader(context) {}

	std::unique_ptr<TextureGpuData> TextureLoader::Load(const std::string& input)
	{
		auto gpuData = std::make_unique<TextureGpuData>();

		stbi_uc* pixels =
			stbi_load(input.c_str(), &gpuData->width, &gpuData->height, &gpuData->channels, STBI_rgb_alpha);

		if (!pixels)
		{
			std::stringstream ss;
			ss << "Failed to load texture image from path: " << input << " - " << stbi_failure_reason() << std::endl;
			throw std::runtime_error(ss.str());
		}

		gpuData->size = gpuData->width * gpuData->height * STBI_rgb_alpha;
		gpuData->mipLevels =
			static_cast<std::uint32_t>(std::floor(std::log2(std::max(gpuData->width, gpuData->height)))) + 1;
		gpuData->image = CreateTextureImage(gpuData->width, gpuData->height, gpuData->mipLevels, gpuData->size, pixels);

		stbi_image_free(pixels);

		gpuData->imageView =
			CreateTextureImageView(gpuData->image->Handle(), VK_FORMAT_R8G8B8A8_UNORM, gpuData->mipLevels);
		gpuData->sampler = CreateTextureSampler(gpuData->mipLevels);
		gpuData->descriptorSet = CreateTextureDescriptorSet(gpuData.get());

		return gpuData;
	}

	std::unique_ptr<VulkanImage> TextureLoader::CreateTextureImage(std::uint32_t width, std::uint32_t height,
																   std::uint32_t mipLevels, std::uint32_t size,
																   void* pixels)
	{
		const VulkanStagingBuffer stagingBuffer(m_context, size);
		stagingBuffer.Write(pixels, size);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage =
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		auto image = std::make_unique<VulkanImage>(m_context, &imageInfo);

		CommandBuffer::SingleTimeCommands(
			[&](const std::unique_ptr<CommandBuffer>& commandBuffer) {
				VkImage vkImage = image->Handle();

				commandBuffer->TransitionImageLayout(vkImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED,
													 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
				commandBuffer->CopyBufferToImage(stagingBuffer.Handle(), vkImage, width, height);

				// Transition to TRANSFER_DST_OPTIMAL is done in mipmap generation _createMipmaps
				// commandBuffer->TransitionImageLayout(vkImage, VK_FORMAT_R8G8B8A8_SRGB,
				// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);
			},
			m_context);

		CreateMipmaps(image->Handle(), VK_FORMAT_R8G8B8A8_UNORM, width, height, mipLevels);

		return image;
	}

	std::unique_ptr<VulkanImageView> TextureLoader::CreateTextureImageView(VkImage image, VkFormat format,
																		   std::uint32_t mipLevels)
	{
		return std::make_unique<VulkanImageView>(m_context, image, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
	}

	void TextureLoader::CreateMipmaps(VkImage image, VkFormat imageFormat, std::int32_t texWidth,
									  std::int32_t texHeight, std::uint32_t mipLevels) const
	{
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(m_context->device->PhysicalHandle(), imageFormat, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
			throw std::runtime_error("texture image format does not support linear blitting!");

		CommandBuffer::SingleTimeCommands(
			[&](const std::unique_ptr<CommandBuffer>& commandBuffer) {
				VkImageMemoryBarrier barrier{};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.image = image;
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount = 1;
				barrier.subresourceRange.levelCount = 1;

				std::int32_t mipWidth = texWidth;
				std::int32_t mipHeight = texHeight;
				for (std::uint32_t i = 1; i < mipLevels; i++)
				{
					barrier.subresourceRange.baseMipLevel = i - 1;
					barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

					commandBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
												   nullptr, 0, nullptr, 1, &barrier);

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

					commandBuffer->BlitImage(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
											 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &blit, VK_FILTER_LINEAR);

					barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

					commandBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
												   VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1,
												   &barrier);

					if (mipWidth > 1)
						mipWidth /= 2;
					if (mipHeight > 1)
						mipHeight /= 2;
				}

				barrier.subresourceRange.baseMipLevel = mipLevels - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				commandBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
											   0, nullptr, 0, nullptr, 1, &barrier);
			},
			m_context);
	}

	std::unique_ptr<VulkanSampler> TextureLoader::CreateTextureSampler(std::uint32_t mipLevels)
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = m_context->device->GetMaxAnisotropy();
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(mipLevels);
		samplerInfo.mipLodBias = 0.0f;

		return std::make_unique<VulkanSampler>(m_context, &samplerInfo);
	}

	std::unique_ptr<VulkanDescriptorSet> TextureLoader::CreateTextureDescriptorSet(const TextureGpuData* textureGpuData)
	{
		auto descriptorSet = std::make_unique<VulkanDescriptorSet>(m_context->vkSamplerDescriptorSetLayout, m_context);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureGpuData->imageView->Handle();
		imageInfo.sampler = textureGpuData->sampler->Handle();

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet->Handle();
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(m_context->device->Handle(), 1, &descriptorWrite, 0, nullptr);

		return descriptorSet;
	}
} // BetterThanNothing
