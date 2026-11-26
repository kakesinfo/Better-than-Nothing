#include <gtest/gtest.h>

#include "Passes/ScreenSpaceShadowPass.hpp"

using namespace BetterThanNothing;

TEST(ScreenSpaceShadowTest, BuildDispatchListCoversOnScreenLight)
{
	const SSSDispatchList list = ScreenSpaceShadowPass::BuildDispatchList(Vector4(0.0f, 0.0f, 0.5f, 1.0f), 1920, 1080, 64);
	EXPECT_GE(list.dispatchCount, 1u);
	EXPECT_LE(list.dispatchCount, 4u);
	for (uint32_t i = 0; i < list.dispatchCount; ++i)
	{
		EXPECT_GT(list.dispatches[i].waveCountX, 0u);
		EXPECT_GT(list.dispatches[i].waveCountY, 0u);
		EXPECT_GT(list.dispatches[i].waveCountZ, 0u);
	}
}

TEST(ScreenSpaceShadowTest, BuildDispatchListHandlesOffScreenAndEdgeLight)
{
	const SSSDispatchList offScreen = ScreenSpaceShadowPass::BuildDispatchList(Vector4(5.0f, -3.0f, 0.5f, 1.0f), 1280, 720, 64);
	EXPECT_EQ(offScreen.dispatchCount, 1u);
	EXPECT_GT(offScreen.dispatches[0].waveCountY, 0u);
	EXPECT_GT(offScreen.dispatches[0].waveCountZ, 0u);

	const SSSDispatchList degenerate = ScreenSpaceShadowPass::BuildDispatchList(Vector4(1.0f, 1.0f, 0.0f, 0.0f), 1280, 720, 64);
	EXPECT_GE(degenerate.dispatchCount, 1u);
	EXPECT_LE(degenerate.dispatchCount, 4u);
}
