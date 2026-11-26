#include <Common/AssetPath.hpp>
#include <ResourceImporter/ResourceImporter.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(AOShadersTest, CompileRTAOComputeShader)
{
	ResourceImporter	 importer;
	const ImportedShader comp = importer.ImportShader(AssetPath::Resolve("Shaders/ao/rtao.comp"));
	EXPECT_EQ(comp.type, EShaderType::Compute);
	EXPECT_FALSE(comp.spirv.empty());
}

TEST(AOShadersTest, CompileGTAOComputeShader)
{
	ResourceImporter	 importer;
	const ImportedShader comp = importer.ImportShader(AssetPath::Resolve("Shaders/ao/gtao.comp"));
	EXPECT_EQ(comp.type, EShaderType::Compute);
	EXPECT_FALSE(comp.spirv.empty());
}
