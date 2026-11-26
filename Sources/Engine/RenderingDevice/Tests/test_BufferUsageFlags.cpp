#include <RenderingDevice/Types.hpp>
#include <gtest/gtest.h>
#include <vulkan/vulkan.h>

using namespace BetterThanNothing;

TEST(BufferUsageFlagsTest, RayTracingFlagsMatchVulkanConstants)
{
	EXPECT_EQ(static_cast<uint32_t>(BufferUsageFlags::ShaderDeviceAddress), static_cast<uint32_t>(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT));
	EXPECT_EQ(static_cast<uint32_t>(BufferUsageFlags::AccelerationStructureBuildInputReadOnly), static_cast<uint32_t>(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR));
	EXPECT_EQ(static_cast<uint32_t>(BufferUsageFlags::AccelerationStructureStorage), static_cast<uint32_t>(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR));
}

TEST(BufferUsageFlagsTest, BitwiseOrAndOperatorsWork)
{
	const BufferUsageFlags combined = BufferUsageFlags::Vertex | BufferUsageFlags::ShaderDeviceAddress | BufferUsageFlags::AccelerationStructureBuildInputReadOnly;
	EXPECT_TRUE(combined & BufferUsageFlags::Vertex);
	EXPECT_TRUE(combined & BufferUsageFlags::ShaderDeviceAddress);
	EXPECT_TRUE(combined & BufferUsageFlags::AccelerationStructureBuildInputReadOnly);
	EXPECT_FALSE(combined & BufferUsageFlags::Index);
}
