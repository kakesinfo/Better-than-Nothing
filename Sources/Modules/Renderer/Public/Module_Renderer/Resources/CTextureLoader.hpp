#pragma once

#include "Module_Renderer/Resources/IResourceLoader.hpp"

namespace BetterThanNothing
{
	struct TextureGpuData;
	struct VulkanContext;
	
	class CVulkanImage;
	class CVulkanImageView;
	class CVulkanSampler;
	class CVulkanDescriptorSet;
	
	class CTextureLoader final : public IResourceLoader<std::string, TextureGpuData>
	{
	public:
		explicit CTextureLoader(VulkanContext* context);

		std::unique_ptr<TextureGpuData> Load(const std::string& input) override;

	private:
		std::unique_ptr<CVulkanImage>			_createTextureImage(std::uint32_t width, std::uint32_t height, std::uint32_t mipLevels, std::uint32_t size, void* pixels);
		std::unique_ptr<CVulkanImageView>		_createTextureImageView(VkImage image, VkFormat format, std::uint32_t mipLevels);
		void									_createMipmaps(VkImage image, VkFormat format, std::int32_t texWidth, std::int32_t texHeight, std::uint32_t mipLevels) const;
		std::unique_ptr<CVulkanSampler>			_createTextureSampler(std::uint32_t mipLevels);
		std::unique_ptr<CVulkanDescriptorSet>	_createTextureDescriptorSet(const TextureGpuData* textureGpuData);
	};
}
