#include <gtest/gtest.h>

#include "Passes/SkyAtmospherePass.hpp"

using namespace BetterThanNothing;

TEST(SkyAtmospherePassTest, DirtyTrackingOnlyRecomputesSkyIblWhenSunOrLuminanceChanges)
{
	SkyLutDirtyState state{};
	EXPECT_TRUE(state.brdfLutDirty);
	EXPECT_TRUE(state.atmosphereStaticLutsDirty);
	EXPECT_TRUE(state.skyIblLutsDirty);

	const Vector3 sunDir(0.0f, -0.7f, -0.7f);
	SkyAtmospherePass::EvaluateDirtyFlags(state, sunDir, 8000.0f);
	EXPECT_TRUE(state.skyIblLutsDirty);

	// Simulate first frame execution clearing dirty flags
	state.brdfLutDirty = false;
	state.atmosphereStaticLutsDirty = false;
	state.skyIblLutsDirty = false;

	// Steady-state frame: 0 LUTs should be dirty
	SkyAtmospherePass::EvaluateDirtyFlags(state, sunDir, 8000.0f);
	EXPECT_FALSE(state.brdfLutDirty);
	EXPECT_FALSE(state.atmosphereStaticLutsDirty);
	EXPECT_FALSE(state.skyIblLutsDirty);

	// Changing sun direction marks only sky/IBL LUTs dirty
	SkyAtmospherePass::EvaluateDirtyFlags(state, Vector3(0.3f, -0.6f, -0.7f), 8000.0f);
	EXPECT_FALSE(state.brdfLutDirty);
	EXPECT_FALSE(state.atmosphereStaticLutsDirty);
	EXPECT_TRUE(state.skyIblLutsDirty);

	state.skyIblLutsDirty = false;
	SkyAtmospherePass::EvaluateDirtyFlags(state, Vector3(0.3f, -0.6f, -0.7f), 12000.0f);
	EXPECT_TRUE(state.skyIblLutsDirty);
}
