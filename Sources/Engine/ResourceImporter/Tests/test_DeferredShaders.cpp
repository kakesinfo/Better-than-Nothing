#include <Common/AssetPath.hpp>
#include <ResourceImporter/ResourceImporter.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(DeferredShadersTest, DeferredLightingComputeCompilesToSpirv)
{
	ResourceImporter importer;
	const ImportedShader comp = importer.ImportShader(AssetPath::Resolve("Shaders/deferred/deferred_lighting.comp"));
	EXPECT_EQ(comp.type, EShaderType::Compute);
	EXPECT_FALSE(comp.spirv.empty());
}
