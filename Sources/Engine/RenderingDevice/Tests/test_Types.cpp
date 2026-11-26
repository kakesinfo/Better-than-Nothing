#include <RenderingDevice/Types.hpp>
#include <gtest/gtest.h>
#include <vulkan/vulkan.h>

#include <set>

using namespace BetterThanNothing;

TEST(RenderingDeviceTypes, FloatTextureFormatsMatchVulkan)
{
	EXPECT_EQ(static_cast<uint32_t>(TextureFormat::R16_SFLOAT), static_cast<uint32_t>(VK_FORMAT_R16_SFLOAT));
	EXPECT_EQ(static_cast<uint32_t>(TextureFormat::R16G16_SFLOAT), static_cast<uint32_t>(VK_FORMAT_R16G16_SFLOAT));
	EXPECT_EQ(static_cast<uint32_t>(TextureFormat::R16G16B16A16_SFLOAT), static_cast<uint32_t>(VK_FORMAT_R16G16B16A16_SFLOAT));
	EXPECT_EQ(static_cast<uint32_t>(TextureFormat::R32_SFLOAT), static_cast<uint32_t>(VK_FORMAT_R32_SFLOAT));
}

TEST(RenderingDeviceTypes, AccelerationStructureDescriptorTypeIsDistinct)
{
	const std::set<uint8_t> values = {
		static_cast<uint8_t>(DescriptorType::UniformBuffer),
		static_cast<uint8_t>(DescriptorType::StorageBuffer),
		static_cast<uint8_t>(DescriptorType::CombinedImageSampler),
		static_cast<uint8_t>(DescriptorType::StorageImage),
		static_cast<uint8_t>(DescriptorType::AccelerationStructure),
	};
	EXPECT_EQ(values.size(), 5u);
}
