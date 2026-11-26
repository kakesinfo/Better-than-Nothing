#include <Data/Graphics/DirectionalLightData.hpp>
#include <Data/Graphics/PointLightData.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(PhotometricLightsTest, DefaultsAndStd430LayoutAreValid)
{
	static_assert(sizeof(PointLightData) == 32);
	static_assert(offsetof(PointLightData, position) == 0);
	static_assert(offsetof(PointLightData, radius) == 12);
	static_assert(offsetof(PointLightData, color) == 16);
	static_assert(offsetof(PointLightData, luminousPowerLm) == 28);

	const DirectionalLightData dir{};
	EXPECT_FLOAT_EQ(dir.illuminanceLux, 100000.0f);

	const PointLightData pt{};
	EXPECT_FLOAT_EQ(pt.luminousPowerLm, 150000.0f);
	EXPECT_FLOAT_EQ(pt.radius, 25.0f);
}
