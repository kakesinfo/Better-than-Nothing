#include <Common/AssetPath.hpp>
#include <ResourceImporter/ResourceImporter.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(SkyShadersTest, CompileTransmittanceLutShader)
{
	ResourceImporter	 importer;
	const ImportedShader comp = importer.ImportShader(AssetPath::Resolve("Shaders/sky/transmittance_lut.comp"));
	EXPECT_EQ(comp.type, EShaderType::Compute);
	EXPECT_FALSE(comp.spirv.empty());
}

TEST(SkyShadersTest, CompileMultiScatteringLutShader)
{
	ResourceImporter	 importer;
	const ImportedShader comp = importer.ImportShader(AssetPath::Resolve("Shaders/sky/multiscattering_lut.comp"));
	EXPECT_EQ(comp.type, EShaderType::Compute);
	EXPECT_FALSE(comp.spirv.empty());
}
