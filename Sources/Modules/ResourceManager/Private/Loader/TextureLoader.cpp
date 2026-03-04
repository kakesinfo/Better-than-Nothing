/*
 *	Better than Nothing
 *	Copyright (C) 2025 kakesinfo
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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "RenderingDevice/RenderingDevice.hpp"
#include "RenderingDevice/SamplerDescription.hpp"
#include "RenderingDevice/TextureDescription.hpp"
#include "ResourceManager/Loader/TextureLoader.hpp"
#include "Module_Base/Logger.hpp"
#include "RenderingApi/Resources/Sampler.hpp"
#include "RenderingApi/Resources/Texture.hpp"
#include "ResourceManager/TextureGpuData.hpp"

#include <RenderingApi.hpp>
#include <sstream>

namespace BetterThanNothing
{
	TextureLoader::TextureLoader(VulkanContext* context, RenderingDevice* renderingDevice)
		: ResourceLoader(context), m_renderingDevice(renderingDevice)
	{}

	std::unique_ptr<TextureGpuData> TextureLoader::Load(const std::string& input)
	{
		auto gpuData = std::make_unique<TextureGpuData>();

		stbi_uc* pixels = stbi_load(input.c_str(), &gpuData->width, &gpuData->height, &gpuData->channels, STBI_rgb_alpha);

		if (!pixels)
		{
			std::stringstream ss;
			ss << "Failed to load texture image from path: " << input << " - " << stbi_failure_reason() << std::endl;
			throw std::runtime_error(ss.str());
		}

		gpuData->channels = STBI_rgb_alpha; // Override with 4 channels
		gpuData->size = gpuData->width * gpuData->height * gpuData->channels;

		TextureFromDataDescription desc{};
		desc.data = pixels;
		desc.dataSize = gpuData->size;
		desc.width = gpuData->width;
		desc.height = gpuData->height;
		desc.channels = gpuData->channels;

		gpuData->textureHandle = m_renderingDevice->CreateTextureFromData(desc);
		gpuData->samplerHandle = m_renderingDevice->GetOrCreateTextureSampler(SamplerDescription{}); // Default sampler description
		gpuData->descriptorSetHandle = m_renderingDevice->CreateTextureDescriptorSet(gpuData->textureHandle, gpuData->samplerHandle);

		stbi_image_free(pixels);

		Logger::Log<INFO>("Loaded Texture {}", input);

		return gpuData;
	}

	std::unique_ptr<VulkanDescriptorSet> TextureLoader::CreateTextureDescriptorSet(const TextureGpuData* textureGpuData)
	{
		auto layout = m_renderingDevice->GetDescriptorSetLayout(DescriptorSetLayoutType::Textures);

		auto descriptorSet = std::make_unique<VulkanDescriptorSet>(layout, m_context);

		auto texture = m_renderingDevice->GetTexture(textureGpuData->textureHandle);
		auto sampler = m_renderingDevice->GetSampler(textureGpuData->samplerHandle);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = texture->defaultView;
		imageInfo.sampler = sampler->value;

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
