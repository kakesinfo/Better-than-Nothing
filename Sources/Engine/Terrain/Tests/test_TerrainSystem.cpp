#include <gtest/gtest.h>

#include <Terrain/TerrainSystem.hpp>
#include <cmath>

using namespace BetterThanNothing;

TEST(TerrainSystemTest, MapTextureStoresOverrideByType)
{
	TerrainSystem::mapTexture(TerrainSystem::GrassTexture, "Assets/Textures/CustomGrass.jpg");
	const auto& overrides = TerrainSystem::GetTextureOverrides();
	EXPECT_EQ(overrides[static_cast<size_t>(TerrainSystem::GrassTexture)], "Assets/Textures/CustomGrass.jpg");
	TerrainSystem::mapTexture(TerrainSystem::GrassTexture, "");
}

TEST(TerrainSystemTest, ComputeTreePositionsRespectsSpacingAndExclusions)
{
	const TerrainSystem		   system;
	const TreeScatterConfig	   cfg{};
	const std::vector<Vector3> positions = system.ComputeTreePositions(cfg);

	EXPECT_EQ(positions.size(), cfg.targetCount);

	const float minSpacingSq = cfg.minSpacing * cfg.minSpacing - 1e-3f;
	for (size_t i = 0; i < positions.size(); ++i)
	{
		const Vector3& p = positions[i];
		const bool	   inExclusion = p.GetX() >= cfg.exclusionMinX && p.GetX() <= cfg.exclusionMaxX && p.GetZ() >= cfg.exclusionMinZ && p.GetZ() <= cfg.exclusionMaxZ;
		EXPECT_FALSE(inExclusion);
		EXPECT_GE(p.GetY(), system.GetGenerator().GetConfig().waterLevel + 0.34f);

		for (size_t j = i + 1; j < positions.size(); ++j)
		{
			const float dx = p.GetX() - positions[j].GetX();
			const float dz = p.GetZ() - positions[j].GetZ();
			EXPECT_GE(dx * dx + dz * dz, minSpacingSq);
		}
	}
}
