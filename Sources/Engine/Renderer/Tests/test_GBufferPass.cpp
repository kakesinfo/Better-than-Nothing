#include "Passes/GBufferPass.hpp"
#include <ResourceManager/PipelineBlueprint.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(GBufferPassTest, NameAndMrtBlueprintSettings)
{
	const GBufferPass pass;
	EXPECT_EQ(pass.GetName(), "GBuffer");

	PipelineBlueprint bp;
	bp.SetUseMsaa(false)
		->SetDepthFormat(TextureFormat::D32_SFLOAT)
		->SetColorFormats({
			TextureFormat::R8G8B8A8_UNORM,
			TextureFormat::R16G16_SFLOAT,
			TextureFormat::R8G8B8A8_UNORM,
			TextureFormat::R16G16_SFLOAT,
		});

	EXPECT_FALSE(bp.GetUseMsaa());
	EXPECT_EQ(bp.GetDepthFormat(), TextureFormat::D32_SFLOAT);
	ASSERT_EQ(bp.GetColorFormats().size(), 4u);
}
