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
