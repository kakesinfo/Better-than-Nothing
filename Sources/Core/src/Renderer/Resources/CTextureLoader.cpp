#include "BetterThanNothing.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace BetterThanNothing
{
	CTextureLoader::CTextureLoader(VulkanContext *context): IResourceLoader(context) {}

	std::unique_ptr<TextureGpuData> CTextureLoader::Load(const std::string& path)
	{
		auto textureGpuData = std::make_unique<TextureGpuData>();

		stbi_uc* pixels = stbi_load(path.c_str(), &textureGpuData->width, &textureGpuData->height, &textureGpuData->channels, STBI_rgb_alpha);
		if (!pixels)
			throw std::runtime_error("failed to load texture image!");

		textureGpuData->size = textureGpuData->width * textureGpuData->height * STBI_rgb_alpha;
		textureGpuData->pVkImage = _createTextureImage(textureGpuData->width, textureGpuData->height, textureGpuData->size, pixels);

		stbi_image_free(pixels);

		textureGpuData->pVkImageView = _createTextureImageView(textureGpuData->pVkImage->Handle(), VK_FORMAT_R8G8B8A8_SRGB);
		textureGpuData->pVkSampler = _createTextureSampler();
		textureGpuData->pVkDescriptorSet = _createTextureDescriptorSet(textureGpuData.get());

		return textureGpuData;
	}

	std::unique_ptr<CVulkanImage> CTextureLoader::_createTextureImage(uint32 width, uint32 height, uint32 size, void* pixels)
	{
		const CVulkanStagingBuffer stagingBuffer(m_context, size);
		stagingBuffer.Write(pixels, size);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		auto pVkImage = std::make_unique<CVulkanImage>(m_context, &imageInfo);

		CVulkanCommandBuffer::SingleTimeCommands([&](const std::unique_ptr<CVulkanCommandBuffer>& commandBuffer) {
			VkImage vkImage = pVkImage->Handle();

			commandBuffer->TransitionImageLayout(vkImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			commandBuffer->CopyBufferToImage(stagingBuffer.Handle(), vkImage, width, height);
			commandBuffer->TransitionImageLayout(vkImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}, m_context);

		return pVkImage;
	}

	std::unique_ptr<CVulkanImageView> CTextureLoader::_createTextureImageView(VkImage image, VkFormat format)
	{
		return std::make_unique<CVulkanImageView>(m_context, image, format, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	std::unique_ptr<CVulkanSampler> CTextureLoader::_createTextureSampler()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = m_context->vkDevice->GetMaxAnisotropy();
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		return std::make_unique<CVulkanSampler>(m_context, &samplerInfo);
	}

	std::unique_ptr<CVulkanDescriptorSet> CTextureLoader::_createTextureDescriptorSet(const TextureGpuData* textureGpuData)
	{
		auto descriptorSet = std::make_unique<CVulkanDescriptorSet>(m_context->vkSamplerDescriptorSetLayout, m_context);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureGpuData->pVkImageView->Handle();
		imageInfo.sampler = textureGpuData->pVkSampler->Handle();

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet->Handle();
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(m_context->vkDevice->Handle(), 1, &descriptorWrite, 0, nullptr);

		return descriptorSet;
	}

} // BetterThanNothing