#include <Renderer/Renderer.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(HaltonJitterTest, ProducesExpectedBase2AndBase3Sequence)
{
	EXPECT_FLOAT_EQ(Renderer::Halton(1, 2), 0.5f);
	EXPECT_FLOAT_EQ(Renderer::Halton(2, 2), 0.25f);
	EXPECT_FLOAT_EQ(Renderer::Halton(3, 2), 0.75f);

	EXPECT_NEAR(Renderer::Halton(1, 3), 1.0f / 3.0f, 1e-6f);
	EXPECT_NEAR(Renderer::Halton(2, 3), 2.0f / 3.0f, 1e-6f);
	EXPECT_NEAR(Renderer::Halton(3, 3), 1.0f / 9.0f, 1e-6f);

	for (uint32_t i = 1; i <= 16; ++i)
	{
		const float hx = Renderer::Halton(i, 2);
		const float hy = Renderer::Halton(i, 3);
		EXPECT_GT(hx, 0.0f);
		EXPECT_LT(hx, 1.0f);
		EXPECT_GT(hy, 0.0f);
		EXPECT_LT(hy, 1.0f);
	}
}
