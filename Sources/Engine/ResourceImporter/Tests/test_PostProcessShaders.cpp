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

TEST(PostProcessShadersTest, TonemapVertexAndFragmentCompileToSpirv)
{
	ResourceImporter importer;

	const ImportedShader vert = importer.ImportShader(AssetPath::Resolve("Shaders/postprocess/tonemap.vert"));
	EXPECT_EQ(vert.type, EShaderType::Vertex);
	EXPECT_FALSE(vert.spirv.empty());

	const ImportedShader frag = importer.ImportShader(AssetPath::Resolve("Shaders/postprocess/tonemap.frag"));
	EXPECT_EQ(frag.type, EShaderType::Fragment);
	EXPECT_FALSE(frag.spirv.empty());
}
