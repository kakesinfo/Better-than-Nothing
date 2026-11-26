#include <gtest/gtest.h>

#include "Passes/ScreenSpaceShadowPass.hpp"

using namespace BetterThanNothing;

TEST(ScreenSpaceShadowTest, BuildDispatchListCoversOnScreenLight)
{
	const SSSDispatchList list = ScreenSpaceShadowPass::BuildDispatchList(Matrix4::Identity, Vector3(0.0f, 0.5f, 0.866f), 1920, 1080);
	EXPECT_GE(list.dispatchCount, 1u);
	EXPECT_LE(list.dispatchCount, 4u);
	for (uint32_t i = 0; i < list.dispatchCount; ++i)
	{
		EXPECT_EQ(list.dispatches[i].waveCountX, 64u);
		EXPECT_GT(list.dispatches[i].waveCountY, 0u);
		EXPECT_GT(list.dispatches[i].waveCountZ, 0u);
	}
}

TEST(ScreenSpaceShadowTest, BuildDispatchListHandlesOffScreenAndEdgeLight)
{
	const SSSDispatchList offScreen = ScreenSpaceShadowPass::BuildDispatchList(Matrix4::Identity, Vector3(1.0f, 0.0f, 0.0f), 1280, 720);
	EXPECT_GE(offScreen.dispatchCount, 1u);
	EXPECT_LE(offScreen.dispatchCount, 4u);
	EXPECT_EQ(offScreen.dispatches[0].waveCountX, 64u);
	EXPECT_GT(offScreen.dispatches[0].waveCountY, 0u);
	EXPECT_GT(offScreen.dispatches[0].waveCountZ, 0u);

	const SSSDispatchList empty = ScreenSpaceShadowPass::BuildDispatchList(Matrix4::Identity, Vector3(0.0f, 1.0f, 0.0f), 0, 0);
	EXPECT_EQ(empty.dispatchCount, 0u);
}
