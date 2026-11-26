#include <Common/AssetPath.hpp>
#include <ResourceImporter/ResourceImporter.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(PostProcessShadersTest, TaaComputeCompilesToSpirv)
{
	ResourceImporter importer;
	const ImportedShader comp = importer.ImportShader(AssetPath::Resolve("Shaders/postprocess/taa.comp"));
	EXPECT_EQ(comp.type, EShaderType::Compute);
	EXPECT_FALSE(comp.spirv.empty());
}
