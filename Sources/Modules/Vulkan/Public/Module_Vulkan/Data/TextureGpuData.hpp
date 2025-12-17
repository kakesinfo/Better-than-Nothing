#pragma once

namespace BetterThanNothing
{
	class CVulkanImage;
	class CVulkanImageView;
	class CVulkanSampler;
	class CVulkanDescriptorSet;

	struct TextureGpuData
	{
		std::int32_t							width;
		std::int32_t							height;
		std::int32_t							channels;
		std::int32_t							size;
		std::uint32_t							mipLevels;

		std::unique_ptr<CVulkanImage>			pVkImage;
		std::unique_ptr<CVulkanImageView>		pVkImageView;
		std::unique_ptr<CVulkanSampler>			pVkSampler;

		std::unique_ptr<CVulkanDescriptorSet>	pVkDescriptorSet;
	};
}