#include <RenderingDevice/RenderingDevice.hpp>
#include <gtest/gtest.h>
#include <type_traits>

using namespace BetterThanNothing;

TEST(TlasDesc, DefaultConstructsIdentityTransformAndMask)
{
	const TlasInstanceDesc desc{};

	EXPECT_FLOAT_EQ(desc.transform[0], 1.0f);
	EXPECT_FLOAT_EQ(desc.transform[1], 0.0f);
	EXPECT_FLOAT_EQ(desc.transform[2], 0.0f);
	EXPECT_FLOAT_EQ(desc.transform[3], 0.0f);
	EXPECT_FLOAT_EQ(desc.transform[4], 0.0f);
	EXPECT_FLOAT_EQ(desc.transform[5], 1.0f);
	EXPECT_FLOAT_EQ(desc.transform[6], 0.0f);
	EXPECT_FLOAT_EQ(desc.transform[7], 0.0f);
	EXPECT_FLOAT_EQ(desc.transform[8], 0.0f);
	EXPECT_FLOAT_EQ(desc.transform[9], 0.0f);
	EXPECT_FLOAT_EQ(desc.transform[10], 1.0f);
	EXPECT_FLOAT_EQ(desc.transform[11], 0.0f);
	EXPECT_FALSE(desc.blasHandle.IsValid());
	EXPECT_EQ(desc.instanceCustomIndex, 0u);
	EXPECT_EQ(desc.mask, 0xFFu);
}

TEST(TlasDesc, RenderingDeviceExposesBuildOrUpdateTlas)
{
	static_assert(std::is_same_v<decltype(&RenderingDevice::BuildOrUpdateTlas), void (RenderingDevice::*)(Handle&, Handle&, Handle&, std::span<const TlasInstanceDesc>)>);
	SUCCEED();
}
