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
