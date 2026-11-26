#include <Common/AssetPath.hpp>
#include <ResourceImporter/ResourceImporter.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(ForwardShadersTest, MainVertexAndFragmentCompileToSpirv)
{
	ResourceImporter importer;

	const ImportedShader vert = importer.ImportShader(AssetPath::Resolve("Shaders/main/main.vert"));
	EXPECT_EQ(vert.type, EShaderType::Vertex);
	EXPECT_FALSE(vert.spirv.empty());

	const ImportedShader frag = importer.ImportShader(AssetPath::Resolve("Shaders/main/main.frag"));
	EXPECT_EQ(frag.type, EShaderType::Fragment);
	EXPECT_FALSE(frag.spirv.empty());
}
