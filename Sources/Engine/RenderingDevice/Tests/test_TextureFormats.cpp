#include "RenderingDevice_FormatUtils.hpp"
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(TextureFormatMapping, MapsFloatAndStandardFormatsToVkFormat)
{
	EXPECT_EQ(ToVkFormat(TextureFormat::R16_SFLOAT), VK_FORMAT_R16_SFLOAT);
	EXPECT_EQ(ToVkFormat(TextureFormat::R16G16_SFLOAT), VK_FORMAT_R16G16_SFLOAT);
	EXPECT_EQ(ToVkFormat(TextureFormat::R16G16B16A16_SFLOAT), VK_FORMAT_R16G16B16A16_SFLOAT);
	EXPECT_EQ(ToVkFormat(TextureFormat::R32_SFLOAT), VK_FORMAT_R32_SFLOAT);
	EXPECT_EQ(ToVkFormat(TextureFormat::D32_SFLOAT), VK_FORMAT_D32_SFLOAT);
	EXPECT_EQ(ToVkFormat(TextureFormat::R8G8B8A8_UNORM), VK_FORMAT_R8G8B8A8_UNORM);
	EXPECT_EQ(ToVkFormat(TextureFormat::R8G8B8A8_SRGB), VK_FORMAT_R8G8B8A8_SRGB);
	EXPECT_EQ(ToVkFormat(TextureFormat::B8G8R8A8_UNORM), VK_FORMAT_B8G8R8A8_UNORM);
}

TEST(TextureFormatMapping, UndefinedReturnsFallback)
{
	EXPECT_EQ(ToVkFormat(TextureFormat::Undefined, VK_FORMAT_B8G8R8A8_SRGB), VK_FORMAT_B8G8R8A8_SRGB);
}
