#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class CVulkanImage;

	struct TextureGpuData
	{
		int32									width;
		int32									height;
		int32									channels;
		int32									size;

		std::unique_ptr<CVulkanImage>			pVkImage;
		std::unique_ptr<CVulkanImageView>		pVkImageView;
		std::unique_ptr<CVulkanSampler>			pVkSampler;

		std::unique_ptr<CVulkanDescriptorSet>	pVkDescriptorSet;
	};
}