#include <gtest/gtest.h>

#include <Terrain/TerrainGenerator.hpp>
#include <numeric>

using namespace BetterThanNothing;

TEST(TerrainGeneratorTest, SponzaPlateauIsFlatAndAtTargetHeight)
{
	const TerrainGenerator gen;
	EXPECT_NEAR(gen.SampleHeight(0.0f, 0.0f), -0.18f, 1e-4f);
	EXPECT_NEAR(gen.SampleHeight(-10.0f, 5.0f), -0.18f, 1e-4f);
	EXPECT_NEAR(gen.SampleNormal(0.0f, 0.0f).GetY(), 1.0f, 1e-4f);
}

TEST(TerrainGeneratorTest, ChunkBorderNormalsAreContinuous)
{
	const TerrainGenerator gen;
	const MeshData		   chunkA = gen.GenerateChunkMesh(0, 0);
	const MeshData		   chunkB = gen.GenerateChunkMesh(1, 0);

	const uint32_t stride = gen.GetConfig().quadsPerChunk + 1u;
	ASSERT_EQ(chunkA.vertices.size(), static_cast<size_t>(stride) * stride);
	ASSERT_EQ(chunkB.vertices.size(), static_cast<size_t>(stride) * stride);

	for (uint32_t z = 0; z < stride; ++z)
	{
		const VertexData& rightEdgeA = chunkA.vertices[z * stride + (stride - 1u)];
		const VertexData& leftEdgeB = chunkB.vertices[z * stride];
		EXPECT_NEAR(rightEdgeA.position.GetY(), leftEdgeB.position.GetY(), 1e-5f);
		EXPECT_NEAR(rightEdgeA.normal.GetX(), leftEdgeB.normal.GetX(), 1e-5f);
		EXPECT_NEAR(rightEdgeA.normal.GetY(), leftEdgeB.normal.GetY(), 1e-5f);
		EXPECT_NEAR(rightEdgeA.normal.GetZ(), leftEdgeB.normal.GetZ(), 1e-5f);
	}
}

TEST(TerrainGeneratorTest, SplatWeightsSumToOne)
{
	const TerrainGenerator gen;
	const auto			   w = gen.ComputeSplatWeights(25.0f, 25.0f, 1.5f, 0.1f);
	const float			   sum = std::accumulate(w.begin(), w.end(), 0.0f);
	EXPECT_NEAR(sum, 1.0f, 1e-4f);
}

TEST(TerrainGeneratorTest, WaterChunkOnlyEmitsNearBasins)
{
	const TerrainGenerator gen;
	// Chunk (1, 0) covers [32..64] x [0..32], containing the east lake at (48, 0)
	const MeshData lakeWater = gen.GenerateWaterChunkMesh(1, 0);
	EXPECT_FALSE(lakeWater.vertices.empty());
	EXPECT_FALSE(lakeWater.indices.empty());
	for (const auto& v : lakeWater.vertices)
		EXPECT_FLOAT_EQ(v.position.GetY(), gen.GetConfig().waterLevel);
}

TEST(TerrainGeneratorTest, GrassChunkExcludesSponzaAndUnderwater)
{
	const TerrainGenerator gen;
	const MeshData		   grass = gen.GenerateGrassChunkMesh(0, 0);
	EXPECT_FALSE(grass.vertices.empty());
	EXPECT_EQ(grass.vertices.size() % 5u, 0u);
	EXPECT_EQ(grass.indices.size() % 9u, 0u);

	const auto& sponza = gen.GetConfig().sponzaPlateau;
	for (size_t i = 0; i < grass.vertices.size(); i += 5u)
	{
		const Vector3& basePos = grass.vertices[i].position;
		const bool	   insideSponza = basePos.GetX() >= sponza.minX && basePos.GetX() <= sponza.maxX && basePos.GetZ() >= sponza.minZ && basePos.GetZ() <= sponza.maxZ;
		EXPECT_FALSE(insideSponza);
		EXPECT_GE(basePos.GetY(), gen.GetConfig().waterLevel + 0.10f);
	}
}
