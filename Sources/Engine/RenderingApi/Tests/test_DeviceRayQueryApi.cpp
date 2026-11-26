#include <RenderingApi/Core/Device.hpp>
#include <gtest/gtest.h>
#include <type_traits>

using namespace BetterThanNothing::RenderingApi;

TEST(DeviceRayQueryApi, ExposesAccelerationStructureAndBdaMethods)
{
	static_assert(std::is_same_v<decltype(&Device::HasAccelerationStructureFunctions), bool (Device::*)() const>);
	static_assert(std::is_same_v<decltype(&Device::GetBufferDeviceAddress), VkDeviceAddress (Device::*)(VkBuffer) const>);
	static_assert(std::is_same_v<decltype(&Device::CreateAccelerationStructure), VkResult (Device::*)(const VkAccelerationStructureCreateInfoKHR&, VkAccelerationStructureKHR*) const>);
	static_assert(std::is_same_v<decltype(&Device::DestroyAccelerationStructure), void (Device::*)(VkAccelerationStructureKHR) const>);
	static_assert(std::is_same_v<decltype(&Device::GetAccelerationStructureBuildSizes), void (Device::*)(VkAccelerationStructureBuildTypeKHR, const VkAccelerationStructureBuildGeometryInfoKHR&, const uint32_t*, VkAccelerationStructureBuildSizesInfoKHR*) const>);
	static_assert(std::is_same_v<decltype(&Device::GetAccelerationStructureDeviceAddress), VkDeviceAddress (Device::*)(VkAccelerationStructureKHR) const>);
	static_assert(std::is_same_v<decltype(&Device::CmdBuildAccelerationStructures), void (Device::*)(VkCommandBuffer, uint32_t, const VkAccelerationStructureBuildGeometryInfoKHR*, const VkAccelerationStructureBuildRangeInfoKHR* const*) const>);
	SUCCEED();
}
