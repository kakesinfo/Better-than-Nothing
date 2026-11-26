#include <RenderingDevice/RenderingDevice.hpp>
#include <gtest/gtest.h>
#include <type_traits>

using namespace BetterThanNothing;

TEST(ComputeCommands, RenderingDeviceExposesComputeAndCopyMethods)
{
	static_assert(std::is_same_v<decltype(&RenderingDevice::ComputeBarrier), void (RenderingDevice::*)()>);
	static_assert(std::is_same_v<decltype(&RenderingDevice::CopyTexture), void (RenderingDevice::*)(const Handle&, const Handle&)>);
	static_assert(std::is_same_v<decltype(&RenderingDevice::PushComputeConstants), void (RenderingDevice::*)(std::string_view, const void*, uint32_t)>);
	SUCCEED();
}
