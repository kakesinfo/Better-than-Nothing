#include <gtest/gtest.h>

#include <Terrain/TerrainConfig.hpp>

using namespace BetterThanNothing;

TEST(TerrainConfigTest, SponzaLandscapeDefaultsAreConsistent)
{
	const TerrainConfig cfg = TerrainConfig::CreateSponzaLandscapeDefaults();
	EXPECT_FLOAT_EQ(cfg.sponzaPlateau.targetHeight, -0.18f);
	EXPECT_FLOAT_EQ(cfg.waterLevel, -1.60f);
	EXPECT_EQ(cfg.waterBasins.size(), 4u);
	EXPECT_EQ(cfg.quadsPerChunk, 32u);
	EXPECT_FLOAT_EQ(cfg.chunkSize, 32.0f);
	for (const auto& layer : cfg.layers)
	{
		EXPECT_FALSE(layer.albedoPath.empty());
		EXPECT_FALSE(layer.normalPath.empty());
		EXPECT_FALSE(layer.roughnessPath.empty());
	}

	const TreeScatterConfig treeCfg{};
	EXPECT_EQ(treeCfg.targetCount, 95u);
	EXPECT_FLOAT_EQ(treeCfg.minSpacing, 8.5f);
	EXPECT_LT(treeCfg.exclusionMinX, treeCfg.exclusionMaxX);
	EXPECT_LT(treeCfg.exclusionMinZ, treeCfg.exclusionMaxZ);
}
