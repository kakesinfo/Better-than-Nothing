#include <Common/AssetPath.hpp>
#include <ResourceImporter/Importer.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>

using namespace BetterThanNothing;

TEST(GltfImporterTest, PreTransformMergesTreeElmByMaterialAndConvertsFoliageToMask)
{
	const std::string resolved = AssetPath::Resolve(".assets/tree_elm/scene.gltf");
	if (!std::filesystem::exists(resolved))
		GTEST_SKIP() << "tree_elm asset not found at " << resolved;

	GltfImporter	  importer;
	const ImportedModel model = importer.Import({resolved, true});

	EXPECT_EQ(model.nodes.size(), 1u);
	EXPECT_EQ(model.rootNodes.size(), 1u);
	EXPECT_EQ(model.meshes.size(), 3u);

	const bool hasMask = std::ranges::any_of(model.materials, [](const ImportedMaterial& mat) {
		return mat.alphaMode == EImportedAlphaMode::Mask;
	});
	EXPECT_TRUE(hasMask);
}
