#include <RenderingApi/Core/CommandBuffer.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing::RenderingApi;

TEST(CopyImage, BuildsColorCopyRegion)
{
	const VkImageCopy region = CommandBuffer::MakeFullImageCopyRegion(
		VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 1920, 1080
	);

	EXPECT_EQ(region.srcSubresource.aspectMask, VK_IMAGE_ASPECT_COLOR_BIT);
	EXPECT_EQ(region.srcSubresource.mipLevel, 0u);
	EXPECT_EQ(region.srcSubresource.baseArrayLayer, 0u);
	EXPECT_EQ(region.srcSubresource.layerCount, 1u);
	EXPECT_EQ(region.dstSubresource.aspectMask, VK_IMAGE_ASPECT_COLOR_BIT);
	EXPECT_EQ(region.dstSubresource.mipLevel, 0u);
	EXPECT_EQ(region.dstSubresource.baseArrayLayer, 0u);
	EXPECT_EQ(region.dstSubresource.layerCount, 1u);
	EXPECT_EQ(region.srcOffset.x, 0);
	EXPECT_EQ(region.srcOffset.y, 0);
	EXPECT_EQ(region.srcOffset.z, 0);
	EXPECT_EQ(region.dstOffset.x, 0);
	EXPECT_EQ(region.dstOffset.y, 0);
	EXPECT_EQ(region.dstOffset.z, 0);
	EXPECT_EQ(region.extent.width, 1920u);
	EXPECT_EQ(region.extent.height, 1080u);
	EXPECT_EQ(region.extent.depth, 1u);
}

TEST(CopyImage, BuildsDepthCopyRegion)
{
	const VkImageCopy region = CommandBuffer::MakeFullImageCopyRegion(
		VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, 1280, 720
	);

	EXPECT_EQ(region.srcSubresource.aspectMask, VK_IMAGE_ASPECT_DEPTH_BIT);
	EXPECT_EQ(region.dstSubresource.aspectMask, VK_IMAGE_ASPECT_DEPTH_BIT);
	EXPECT_EQ(region.extent.width, 1280u);
	EXPECT_EQ(region.extent.height, 720u);
	EXPECT_EQ(region.extent.depth, 1u);
}
