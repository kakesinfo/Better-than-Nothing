#include <Common/AssetPath.hpp>
#include <ResourceImporter/ResourceImporter.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(RTShadowShadersTest, CompileRTShadowsComputeShader)
{
	ResourceImporter	 importer;
	const ImportedShader comp = importer.ImportShader(AssetPath::Resolve("Shaders/rtshadows/rt_shadows.comp"));
	EXPECT_EQ(comp.type, EShaderType::Compute);
	EXPECT_FALSE(comp.spirv.empty());
}

TEST(RTShadowShadersTest, CompileFFXTileClassificationComputeShader)
{
	ResourceImporter	 importer;
	const ImportedShader comp = importer.ImportShader(AssetPath::Resolve("Shaders/rtshadows/ffx_shadows_tile_classification.comp"));
	EXPECT_EQ(comp.type, EShaderType::Compute);
	EXPECT_FALSE(comp.spirv.empty());
}

TEST(RTShadowShadersTest, CompileFFXFilterComputeShader)
{
	ResourceImporter	 importer;
	const ImportedShader comp = importer.ImportShader(AssetPath::Resolve("Shaders/rtshadows/ffx_shadows_filter.comp"));
	EXPECT_EQ(comp.type, EShaderType::Compute);
	EXPECT_FALSE(comp.spirv.empty());
}
