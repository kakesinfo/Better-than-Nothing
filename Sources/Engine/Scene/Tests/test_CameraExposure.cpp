#include <Scene/Camera.hpp>
#include <gtest/gtest.h>

#include <cmath>

using namespace BetterThanNothing;

TEST(CameraExposureTest, DefaultSunny16GivesExpectedEV100AndExposure)
{
	const Camera cam(60.0f, 0.1f, 1000.0f);

	EXPECT_FLOAT_EQ(cam.GetAperture(), 11.0f);
	EXPECT_FLOAT_EQ(cam.GetShutterSpeed(), 1.0f / 125.0f);
	EXPECT_FLOAT_EQ(cam.GetISO(), 100.0f);
	EXPECT_FLOAT_EQ(cam.GetEVCompensation(), 0.0f);

	const float ev100 = cam.GetEV100();
	EXPECT_NEAR(ev100, 13.8846f, 0.01f);

	const float expectedExposure = 1.0f / (1.2f * std::exp2(ev100));
	EXPECT_NEAR(cam.GetExposure(), expectedExposure, 1e-8f);
}

TEST(CameraExposureTest, SetPhysicalExposureUpdatesEV100)
{
	Camera cam(60.0f, 0.1f, 1000.0f);
	cam.SetPhysicalExposure(2.8f, 1.0f / 60.0f, 200.0f, 1.0f);

	// EV100 = log2((2.8^2 / (1/60)) * (100 / 200)) - 1.0 = log2(235.2) - 1.0 ~= 6.8777
	EXPECT_NEAR(cam.GetEV100(), std::log2(235.2f) - 1.0f, 1e-3f);
}
