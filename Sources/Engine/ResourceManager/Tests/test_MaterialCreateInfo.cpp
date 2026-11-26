#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceManager_Types.hpp>
#include <gtest/gtest.h>
#include <type_traits>

using namespace BetterThanNothing;

TEST(MaterialCreateInfoTest, DefaultsAndCacheFieldsAreValid)
{
	const MaterialCreateInfo info{};
	EXPECT_EQ(info.diffuseTextureId, InvalidResourceId);
	EXPECT_EQ(info.normalTextureId, InvalidResourceId);
	EXPECT_EQ(info.metallicRoughnessTextureId, InvalidResourceId);
	EXPECT_FLOAT_EQ(info.baseColorFactor.GetX(), 1.0f);
	EXPECT_FLOAT_EQ(info.baseColorFactor.GetY(), 1.0f);
	EXPECT_FLOAT_EQ(info.baseColorFactor.GetZ(), 1.0f);
	EXPECT_FLOAT_EQ(info.baseColorFactor.GetW(), 1.0f);
	EXPECT_FLOAT_EQ(info.metallicFactor, 0.0f);
	EXPECT_FLOAT_EQ(info.roughnessFactor, 1.0f);
	EXPECT_FLOAT_EQ(info.alphaCutoff, 0.5f);

	const MaterialResourceCache cache{};
	EXPECT_EQ(cache.metallicRoughnessTextureId, InvalidResourceId);
	EXPECT_FALSE(cache.uniformBufferHandle.IsValid());

	static_assert(std::is_same_v<decltype(&ResourceManager::GetDefaultMetallicRoughnessTexture), ResourceId (ResourceManager::*)()>);
}
