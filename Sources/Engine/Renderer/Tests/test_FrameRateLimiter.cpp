#include <gtest/gtest.h>

#include <Common/RenderSettings.hpp>

using namespace BetterThanNothing;

TEST(FrameRateLimiterTest, ComputeFrameSleepDurationHandlesUnlimitedAndBudgetedFrames)
{
	using namespace std::chrono_literals;

	EXPECT_EQ(RenderSettings::ComputeFrameSleepDuration(5000us, 0), 0us);
	EXPECT_EQ(RenderSettings::ComputeFrameSleepDuration(5000us, -60), 0us);

	const auto sleep60 = RenderSettings::ComputeFrameSleepDuration(5000us, 60);
	EXPECT_EQ(sleep60.count(), 1000000 / 60 - 5000);

	EXPECT_EQ(RenderSettings::ComputeFrameSleepDuration(20000us, 60), 0us);
}
