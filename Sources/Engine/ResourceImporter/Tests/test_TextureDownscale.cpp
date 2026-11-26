#include <ResourceImporter/Importer.hpp>
#include <gtest/gtest.h>

#include <vector>

using namespace BetterThanNothing;

TEST(TextureDownscaleTest, DownsampleHalfRGAverages2x2Block)
{
	// 4x4 RGBA image: top-left 2x2 has values (10, 20, 30, 40) avg = (25, 25, 25, 25)
	std::vector<uint8_t> src(4 * 4 * 4, 0);
	auto setPixel = [&](const uint32_t x, const uint32_t y, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
		const size_t idx = (y * 4 + x) * 4;
		src[idx + 0] = r;
		src[idx + 1] = g;
		src[idx + 2] = b;
		src[idx + 3] = a;
	};

	setPixel(0, 0, 10, 100, 0, 255);
	setPixel(1, 0, 20, 100, 0, 255);
	setPixel(0, 1, 30, 100, 0, 255);
	setPixel(1, 1, 40, 100, 0, 255);

	std::vector<uint8_t> dst;
	uint32_t			 dstW = 0;
	uint32_t			 dstH = 0;
	DownsampleHalfRGBA(src.data(), 4, 4, dst, dstW, dstH);

	EXPECT_EQ(dstW, 2u);
	EXPECT_EQ(dstH, 2u);
	ASSERT_EQ(dst.size(), 2u * 2u * 4u);
	EXPECT_EQ(dst[0], 25u);
	EXPECT_EQ(dst[1], 100u);
	EXPECT_EQ(dst[2], 0u);
	EXPECT_EQ(dst[3], 255u);
}

TEST(TextureDownscaleTest, HandlesOddDimensionsSafely)
{
	std::vector<uint8_t> src(3 * 3 * 4, 128);
	std::vector<uint8_t> dst;
	uint32_t			 dstW = 0;
	uint32_t			 dstH = 0;
	DownsampleHalfRGBA(src.data(), 3, 3, dst, dstW, dstH);

	EXPECT_EQ(dstW, 1u);
	EXPECT_EQ(dstH, 1u);
	ASSERT_EQ(dst.size(), 4u);
	EXPECT_EQ(dst[0], 128u);
}
