#include <RenderingDevice/Handle.hpp>
#include <RenderingDevice/PipelineDescription.hpp>
#include <ResourceManager/PipelineBlueprint.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(PipelineBlueprintMrt, DefaultsToEmptyColorFormats)
{
	PipelineBlueprint blueprint;
	EXPECT_TRUE(blueprint.GetColorFormats().empty());

	PipelineDescription desc{};
	EXPECT_TRUE(desc.colorFormats.empty());
}

TEST(PipelineBlueprintMrt, SetColorFormatsStoresAndChainsFourTargets)
{
	PipelineBlueprint blueprint;
	const std::vector<TextureFormat> mrtFormats = {
		TextureFormat::R8G8B8A8_UNORM,
		TextureFormat::R16G16_SFLOAT,
		TextureFormat::R8G8B8A8_UNORM,
		TextureFormat::R16G16_SFLOAT,
	};

	PipelineBlueprint* chained = blueprint.SetColorFormats(mrtFormats);
	EXPECT_EQ(chained, &blueprint);
	ASSERT_EQ(blueprint.GetColorFormats().size(), 4u);
	EXPECT_EQ(blueprint.GetColorFormats(), mrtFormats);
}
