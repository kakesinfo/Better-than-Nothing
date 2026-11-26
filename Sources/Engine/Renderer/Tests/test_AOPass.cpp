#include <gtest/gtest.h>

#include "Passes/AOPass.hpp"

using namespace BetterThanNothing;

TEST(AOPassTest, ResolveEffectiveModeFallsBackToGTAOWhenRayQueryOrTlasUnavailable)
{
	EXPECT_EQ(AOPass::ResolveEffectiveMode(EAOMode::Off, true, true), EAOMode::Off);
	EXPECT_EQ(AOPass::ResolveEffectiveMode(EAOMode::RTAO, true, true), EAOMode::RTAO);
	EXPECT_EQ(AOPass::ResolveEffectiveMode(EAOMode::RTAO, false, true), EAOMode::GTAO);
	EXPECT_EQ(AOPass::ResolveEffectiveMode(EAOMode::RTAO, true, false), EAOMode::GTAO);
	EXPECT_EQ(AOPass::ResolveEffectiveMode(EAOMode::GTAO, true, true), EAOMode::GTAO);
}

TEST(AOPassTest, ComputeDispatchGroupsCoversFullScreen)
{
	const auto [gx0, gy0] = AOPass::ComputeDispatchGroups(0, 0);
	EXPECT_EQ(gx0, 0u);
	EXPECT_EQ(gy0, 0u);

	const auto [gx1, gy1] = AOPass::ComputeDispatchGroups(1920, 1080);
	EXPECT_EQ(gx1, 240u);
	EXPECT_EQ(gy1, 135u);

	const auto [gx2, gy2] = AOPass::ComputeDispatchGroups(1921, 1081);
	EXPECT_EQ(gx2, 241u);
	EXPECT_EQ(gy2, 136u);
}
