#include <gtest/gtest.h>

#include "Passes/RTShadowPass.hpp"

using namespace BetterThanNothing;

TEST(RTShadowPassTest, ComputeDispatchGroupsCoversFullScreen)
{
	const auto [gx0, gy0] = RTShadowPass::ComputeDispatchGroups(0, 0);
	EXPECT_EQ(gx0, 0u);
	EXPECT_EQ(gy0, 0u);

	const auto [gx1, gy1] = RTShadowPass::ComputeDispatchGroups(1920, 1080);
	EXPECT_EQ(gx1, 240u);
	EXPECT_EQ(gy1, 270u);

	const auto [gx2, gy2] = RTShadowPass::ComputeDispatchGroups(1921, 1081);
	EXPECT_EQ(gx2, 241u);
	EXPECT_EQ(gy2, 271u);
}

TEST(RTShadowPassTest, FilterStepSizesArePowersOfTwo)
{
	constexpr auto steps = RTShadowPass::GetFilterStepSizes();
	ASSERT_EQ(steps.size(), 3u);
	EXPECT_EQ(steps[0], 1);
	EXPECT_EQ(steps[1], 2);
	EXPECT_EQ(steps[2], 4);
}
