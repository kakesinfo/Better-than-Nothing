#include <Renderer/RenderSettings.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(RenderSettingsTest, ReturnsSameSingletonInstanceWithExpectedDefaults)
{
	RenderSettingsData& a = RenderSettings::Get();
	RenderSettingsData& b = RenderSettings::Get();
	EXPECT_EQ(&a, &b);

	EXPECT_EQ(a.debugView, EDebugViewMode::Final);
	EXPECT_EQ(a.toneMapMode, EToneMapMode::AgX);
	EXPECT_EQ(a.aoMode, EAOMode::RTAO);
	EXPECT_TRUE(a.enableTAA);
	EXPECT_TRUE(a.enableRTShadows);
	EXPECT_TRUE(a.enableScreenSpaceShadows);
	EXPECT_TRUE(a.enableClouds);
	EXPECT_EQ(a.targetFps, 60);
}
