#include <gtest/gtest.h>

#include <Terrain/Noise.hpp>

using namespace BetterThanNothing;

TEST(NoiseTest, QuinticSmoothstepBoundaryAndMidpoint)
{
	EXPECT_FLOAT_EQ(QuinticSmoothstep(-1.0f), 0.0f);
	EXPECT_FLOAT_EQ(QuinticSmoothstep(0.0f), 0.0f);
	EXPECT_FLOAT_EQ(QuinticSmoothstep(0.5f), 0.5f);
	EXPECT_FLOAT_EQ(QuinticSmoothstep(1.0f), 1.0f);
	EXPECT_FLOAT_EQ(QuinticSmoothstep(2.0f), 1.0f);
}

TEST(NoiseTest, GradientFbmAndRidgedAreDeterministicAndBounded)
{
	EXPECT_EQ(Hash2D(12, -7, 42u), Hash2D(12, -7, 42u));
	EXPECT_NE(Hash2D(12, -7, 42u), Hash2D(13, -7, 42u));

	const float g1 = GradientNoise2D(1.37f, -2.84f, 7u);
	const float g2 = GradientNoise2D(1.37f, -2.84f, 7u);
	EXPECT_FLOAT_EQ(g1, g2);
	EXPECT_GE(g1, -1.0f);
	EXPECT_LE(g1, 1.0f);

	const float fbm = FBM2D(0.25f, 0.75f, 4, 2.0f, 0.5f, 11u);
	EXPECT_GE(fbm, -1.0f);
	EXPECT_LE(fbm, 1.0f);

	const float ridged = RidgedFBM2D(0.25f, 0.75f, 4, 2.0f, 0.5f, 11u);
	EXPECT_GE(ridged, 0.0f);
	EXPECT_LE(ridged, 1.0f);
}

TEST(NoiseTest, SeededRngProducesDeterministicBoundedValues)
{
	SeededRng rngA(999u);
	SeededRng rngB(999u);
	for (int i = 0; i < 64; ++i)
	{
		const float a = rngA.NextRange(-5.0f, 10.0f);
		const float b = rngB.NextRange(-5.0f, 10.0f);
		EXPECT_FLOAT_EQ(a, b);
		EXPECT_GE(a, -5.0f);
		EXPECT_LE(a, 10.0f);
	}
}
