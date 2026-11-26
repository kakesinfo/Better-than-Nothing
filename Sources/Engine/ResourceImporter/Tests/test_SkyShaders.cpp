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

TEST(SkyShadersTest, CompileSkyViewLutShader)
{
	ResourceImporter	 importer;
	const ImportedShader comp = importer.ImportShader(AssetPath::Resolve("Shaders/sky/skyview_lut.comp"));
	EXPECT_EQ(comp.type, EShaderType::Compute);
	EXPECT_FALSE(comp.spirv.empty());
}

TEST(SkyShadersTest, CompileBrdfLutShader)
{
	ResourceImporter	 importer;
	const ImportedShader comp = importer.ImportShader(AssetPath::Resolve("Shaders/sky/brdf_lut.comp"));
	EXPECT_EQ(comp.type, EShaderType::Compute);
	EXPECT_FALSE(comp.spirv.empty());
}

TEST(SkyShadersTest, CompileIrradianceAndPrefilterShaders)
{
	ResourceImporter	 importer;
	const ImportedShader irr = importer.ImportShader(AssetPath::Resolve("Shaders/sky/irradiance_convolve.comp"));
	EXPECT_EQ(irr.type, EShaderType::Compute);
	EXPECT_FALSE(irr.spirv.empty());

	const ImportedShader pref = importer.ImportShader(AssetPath::Resolve("Shaders/sky/prefilter_env.comp"));
	EXPECT_EQ(pref.type, EShaderType::Compute);
	EXPECT_FALSE(pref.spirv.empty());
}

TEST(SkyShadersTest, CompileSkyRenderShader)
{
	ResourceImporter	 importer;
	const ImportedShader comp = importer.ImportShader(AssetPath::Resolve("Shaders/sky/sky_render.comp"));
	EXPECT_EQ(comp.type, EShaderType::Compute);
	EXPECT_FALSE(comp.spirv.empty());
}
