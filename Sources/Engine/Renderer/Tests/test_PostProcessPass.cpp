#include "Passes/PostProcessPass.hpp"
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(PostProcessPassTest, NameAndPingPongIndexAlternation)
{
	const PostProcessPass pass;
	EXPECT_EQ(pass.GetName(), "PostProcess");

	uint32_t readIdx = 0u;
	uint32_t writeIdx = 1u - readIdx;
	EXPECT_EQ(writeIdx, 1u);

	readIdx = 1u - readIdx;
	writeIdx = 1u - readIdx;
	EXPECT_EQ(readIdx, 1u);
	EXPECT_EQ(writeIdx, 0u);
}
