#include <ResourceImporter/Importer.hpp>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

using namespace BetterThanNothing;

TEST(ObjImporterTest, ExtractsNormalRoughnessAndAlphaMask)
{
	const std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "btn_obj_importer_test";
	std::filesystem::create_directories(tempDir);

	const std::filesystem::path mtlPath = tempDir / "test_mat.mtl";
	const std::filesystem::path objPath = tempDir / "test_mesh.obj";

	{
		std::ofstream mtl(mtlPath);
		mtl << "newmtl LeafMat\n";
		mtl << "Kd 0.8 0.9 0.7\n";
		mtl << "d 0.8\n";
		mtl << "map_Kd albedo.png\n";
		mtl << "map_Bump norm_ddn.png\n";
		mtl << "map_Ns rough.png\n";
		mtl << "map_d alpha.png\n";
	}

	{
		std::ofstream obj(objPath);
		obj << "mtllib test_mat.mtl\n";
		obj << "o Quad\n";
		obj << "v 0 0 0\n";
		obj << "v 1 0 0\n";
		obj << "v 0 1 0\n";
		obj << "vt 0 0\n";
		obj << "vt 1 0\n";
		obj << "vt 0 1\n";
		obj << "vn 0 0 1\n";
		obj << "usemtl LeafMat\n";
		obj << "f 1/1/1 2/2/1 3/3/1\n";
	}

	ObjImporter		  importer;
	const ImportedModel model = importer.Import({objPath.string(), true});

	std::filesystem::remove_all(tempDir);

	ASSERT_EQ(model.materials.size(), 1u);
	const auto& mat = model.materials[0];
	ASSERT_TRUE(mat.diffuseTexturePath.has_value());
	ASSERT_TRUE(mat.normalTexturePath.has_value());
	ASSERT_TRUE(mat.metallicRoughnessTexturePath.has_value());
	EXPECT_NE(mat.normalTexturePath->find("norm_ddn.png"), std::string::npos);
	EXPECT_NE(mat.metallicRoughnessTexturePath->find("rough.png"), std::string::npos);
	EXPECT_EQ(mat.alphaMode, EImportedAlphaMode::Mask);
	EXPECT_FLOAT_EQ(mat.alphaCutoff, 0.5f);
	EXPECT_NEAR(mat.baseColorFactor.GetW(), 0.8f, 1e-4f);
}
