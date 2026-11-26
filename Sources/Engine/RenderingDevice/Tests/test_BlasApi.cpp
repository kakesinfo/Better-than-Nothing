#include "RenderingDevice_FormatUtils.hpp"
#include <RenderingDevice/RenderingDevice.hpp>
#include <gtest/gtest.h>
#include <type_traits>

using namespace BetterThanNothing;

TEST(BlasApi, AlignUpDeviceSizeAlignsProperly)
{
	EXPECT_EQ(AlignUpDeviceSize(0, 256), 0u);
	EXPECT_EQ(AlignUpDeviceSize(1, 256), 256u);
	EXPECT_EQ(AlignUpDeviceSize(256, 256), 256u);
	EXPECT_EQ(AlignUpDeviceSize(257, 256), 512u);
	EXPECT_EQ(AlignUpDeviceSize(128, 1), 128u);
}

TEST(BlasApi, RenderingDeviceExposesBlasMethods)
{
	static_assert(std::is_same_v<decltype(&RenderingDevice::IsRayQuerySupported), bool (RenderingDevice::*)() const>);
	static_assert(std::is_same_v<decltype(&RenderingDevice::CreateBlas), Handle (RenderingDevice::*)(const Handle&, uint32_t, uint32_t, const Handle&, uint32_t)>);
	static_assert(std::is_same_v<decltype(&RenderingDevice::DestroyAccelerationStructure), void (RenderingDevice::*)(const Handle&)>);
	static_assert(std::is_same_v<decltype(&RenderingDevice::GetAccelerationStructure), RenderingApi::AccelerationStructure* (RenderingDevice::*)(const Handle&)>);
	SUCCEED();
}
