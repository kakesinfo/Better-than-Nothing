#include <Common/AssetPath.hpp>
#include <ResourceImporter/ResourceImporter.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(TerrainShadersTest, CompileTerrainGBufferFragShader)
{
	ResourceImporter	 importer;
	const ImportedShader frag = importer.ImportShader(AssetPath::Resolve("Shaders/gbuffer/terrain_gbuffer.frag"));
	EXPECT_EQ(frag.type, EShaderType::Fragment);
	EXPECT_FALSE(frag.spirv.empty());
}

TEST(TerrainShadersTest, CompileGrassGBufferShaders)
{
	ResourceImporter	 importer;
	const ImportedShader vert = importer.ImportShader(AssetPath::Resolve("Shaders/gbuffer/grass_gbuffer.vert"));
	EXPECT_EQ(vert.type, EShaderType::Vertex);
	EXPECT_FALSE(vert.spirv.empty());

	const ImportedShader frag = importer.ImportShader(AssetPath::Resolve("Shaders/gbuffer/grass_gbuffer.frag"));
	EXPECT_EQ(frag.type, EShaderType::Fragment);
	EXPECT_FALSE(frag.spirv.empty());
}
