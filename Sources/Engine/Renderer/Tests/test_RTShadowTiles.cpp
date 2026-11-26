#include <gtest/gtest.h>

#include <Renderer/Renderer.hpp>

using namespace BetterThanNothing;

TEST(RTShadowTilesTest, ComputeTileCountMatches8x4Grid)
{
	EXPECT_EQ(Renderer::ComputeRTShadowTileCount(0, 0), 0u);
	EXPECT_EQ(Renderer::ComputeRTShadowTileCount(1920, 1080), 240u * 270u);
	EXPECT_EQ(Renderer::ComputeRTShadowTileCount(1280, 720), 160u * 180u);
	EXPECT_EQ(Renderer::ComputeRTShadowTileCount(9, 5), 2u * 2u);
	EXPECT_EQ(Renderer::ComputeRTShadowTileCount(8, 4), 1u);
}
