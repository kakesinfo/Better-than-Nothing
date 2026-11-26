#include "Passes/DeferredLightingPass.hpp"
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(DeferredLightingPassTest, NameAndDispatchGroupCalculation)
{
	const DeferredLightingPass pass;
	EXPECT_EQ(pass.GetName(), "DeferredLighting");

	EXPECT_EQ((1280u + 7u) / 8u, 160u);
	EXPECT_EQ((720u + 7u) / 8u, 90u);
	EXPECT_EQ((1920u + 7u) / 8u, 240u);
	EXPECT_EQ((1080u + 7u) / 8u, 135u);
}
