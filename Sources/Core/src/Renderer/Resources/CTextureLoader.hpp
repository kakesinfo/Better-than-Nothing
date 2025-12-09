#pragma once

namespace BetterThanNothing
{
	class CTextureLoader final : public IResourceLoader<std::string, TextureGpuData>
	{
	public:
		explicit CTextureLoader(VulkanContext* context);

		std::unique_ptr<TextureGpuData> Load(const std::string& input) override;

	private:
		std::unique_ptr<CVulkanImage> _createTextureImage(uint32 width, uint32 height, uint32 size, void* pixels);
		std::unique_ptr<CVulkanImageView> _createTextureImageView(VkImage image, VkFormat format);
		std::unique_ptr<CVulkanSampler> _createTextureSampler();
		std::unique_ptr<CVulkanDescriptorSet> _createTextureDescriptorSet(const TextureGpuData* textureGpuData);
	};
}
