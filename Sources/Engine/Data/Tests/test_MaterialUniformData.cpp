#include <Data/Graphics/MaterialUniformData.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(MaterialUniformDataTest, SizeAndDefaultsMatchStd140)
{
	static_assert(sizeof(MaterialUniformData) == 32);
	static_assert(offsetof(MaterialUniformData, baseColorFactor) == 0);
	static_assert(offsetof(MaterialUniformData, metallicFactor) == 16);
	static_assert(offsetof(MaterialUniformData, roughnessFactor) == 20);
	static_assert(offsetof(MaterialUniformData, alphaCutoff) == 24);

	const MaterialUniformData data{};
	EXPECT_FLOAT_EQ(data.baseColorFactor.GetX(), 1.0f);
	EXPECT_FLOAT_EQ(data.baseColorFactor.GetY(), 1.0f);
	EXPECT_FLOAT_EQ(data.baseColorFactor.GetZ(), 1.0f);
	EXPECT_FLOAT_EQ(data.baseColorFactor.GetW(), 1.0f);
	EXPECT_FLOAT_EQ(data.metallicFactor, 0.0f);
	EXPECT_FLOAT_EQ(data.roughnessFactor, 1.0f);
	EXPECT_FLOAT_EQ(data.alphaCutoff, 0.5f);
}
