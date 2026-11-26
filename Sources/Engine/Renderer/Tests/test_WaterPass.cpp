#include <gtest/gtest.h>

#include "Passes/WaterPass.hpp"

using namespace BetterThanNothing;

TEST(WaterPassTest, ShouldExecuteRequiresNonEmptyStreamAndValidTargets)
{
	DrawStream	 stream;
	const Handle validHandle(1u, 1u, 1u);
	const Handle invalidHandle{};

	EXPECT_FALSE(WaterPass::ShouldExecute(nullptr, validHandle, validHandle, validHandle));
	EXPECT_FALSE(WaterPass::ShouldExecute(&stream, validHandle, validHandle, validHandle));

	DrawPacket pkt{};
	stream.Draw(pkt);
	EXPECT_TRUE(WaterPass::ShouldExecute(&stream, validHandle, validHandle, validHandle));
	EXPECT_FALSE(WaterPass::ShouldExecute(&stream, invalidHandle, validHandle, validHandle));
	EXPECT_FALSE(WaterPass::ShouldExecute(&stream, validHandle, invalidHandle, validHandle));
	EXPECT_FALSE(WaterPass::ShouldExecute(&stream, validHandle, validHandle, invalidHandle));
}
