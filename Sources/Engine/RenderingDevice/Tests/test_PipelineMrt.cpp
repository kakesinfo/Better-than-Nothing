#include "RenderingDevice_FormatUtils.hpp"
#include <RenderingDevice/Handle.hpp>
#include <RenderingDevice/PipelineDescription.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(PipelineMrt, FallsBackToSingleColorFormatWhenColorFormatsEmpty)
{
	PipelineDescription desc{};
	desc.colorFormat = TextureFormat::R16G16B16A16_SFLOAT;

	const auto formats = ResolvePipelineColorFormats(desc, VK_FORMAT_B8G8R8A8_SRGB);
	ASSERT_EQ(formats.size(), 1u);
	EXPECT_EQ(formats[0], VK_FORMAT_R16G16B16A16_SFLOAT);
}

TEST(PipelineMrt, UsesSwapchainFallbackWhenSingleColorFormatUndefined)
{
	PipelineDescription desc{};
	desc.colorFormat = TextureFormat::Undefined;

	const auto formats = ResolvePipelineColorFormats(desc, VK_FORMAT_B8G8R8A8_SRGB);
	ASSERT_EQ(formats.size(), 1u);
	EXPECT_EQ(formats[0], VK_FORMAT_B8G8R8A8_SRGB);
}

TEST(PipelineMrt, ResolvesFourMrtColorFormats)
{
	PipelineDescription desc{};
	desc.colorFormats = {
		TextureFormat::R8G8B8A8_UNORM,
		TextureFormat::R16G16_SFLOAT,
		TextureFormat::R8G8B8A8_UNORM,
		TextureFormat::R16G16_SFLOAT,
	};

	const auto formats = ResolvePipelineColorFormats(desc, VK_FORMAT_B8G8R8A8_SRGB);
	ASSERT_EQ(formats.size(), 4u);
	EXPECT_EQ(formats[0], VK_FORMAT_R8G8B8A8_UNORM);
	EXPECT_EQ(formats[1], VK_FORMAT_R16G16_SFLOAT);
	EXPECT_EQ(formats[2], VK_FORMAT_R8G8B8A8_UNORM);
	EXPECT_EQ(formats[3], VK_FORMAT_R16G16_SFLOAT);
}
