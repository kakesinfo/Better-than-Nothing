#include <gtest/gtest.h>

#include <Renderer/Renderer.hpp>

using namespace BetterThanNothing;

TEST(BorderWhiteSamplerTest, DescriptorMatchesBendSSSRequirements)
{
	const SamplerDescription desc = Renderer::MakePointBorderWhiteSamplerDesc();
	EXPECT_EQ(desc.magFilter, SamplerFilter::Nearest);
	EXPECT_EQ(desc.minFilter, SamplerFilter::Nearest);
	EXPECT_EQ(desc.mipmapMode, SamplerMipmapMode::Nearest);
	EXPECT_EQ(desc.addressModeU, SamplerAddressMode::ClampToBorder);
	EXPECT_EQ(desc.addressModeV, SamplerAddressMode::ClampToBorder);
	EXPECT_EQ(desc.addressModeW, SamplerAddressMode::ClampToBorder);
	EXPECT_FALSE(desc.anisotropyEnable);
	EXPECT_EQ(desc.borderColor, BorderColor::FloatOpaqueWhite);
}
