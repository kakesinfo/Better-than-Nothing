#include <RenderingApi/Core/CommandBuffer.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing::RenderingApi;

TEST(ComputeBarrier, CreatesValidComputeToComputeMemoryBarrier2)
{
	const VkMemoryBarrier2 barrier = CommandBuffer::MakeComputeMemoryBarrier();

	EXPECT_EQ(barrier.sType, VK_STRUCTURE_TYPE_MEMORY_BARRIER_2);
	EXPECT_EQ(barrier.pNext, nullptr);
	EXPECT_EQ(barrier.srcStageMask, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);
	EXPECT_EQ(barrier.dstStageMask, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);
	EXPECT_EQ(barrier.srcAccessMask, VK_ACCESS_2_SHADER_WRITE_BIT);
	EXPECT_EQ(barrier.dstAccessMask, VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT);
}
