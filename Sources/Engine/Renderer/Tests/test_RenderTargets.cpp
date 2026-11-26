#include <Renderer/RenderPass.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(RenderTargetsTest, RenderPassContextExposesGBufferHistoryAndHdrHandles)
{
	RenderPassContext ctx{};
	EXPECT_FALSE(ctx.gbufferAlbedo.IsValid());
	EXPECT_FALSE(ctx.gbufferNormal.IsValid());
	EXPECT_FALSE(ctx.gbufferMaterial.IsValid());
	EXPECT_FALSE(ctx.gbufferMotion.IsValid());
	EXPECT_FALSE(ctx.prevDepthTarget.IsValid());
	EXPECT_FALSE(ctx.prevNormalTarget.IsValid());
	EXPECT_FALSE(ctx.hdrColor.IsValid());
	EXPECT_FALSE(ctx.depthTarget.IsValid());
}
