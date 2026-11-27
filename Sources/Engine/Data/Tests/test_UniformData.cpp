#include <Data/Graphics/DynamicUniformData.hpp>
#include <Data/Graphics/GlobalUniformData.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(UniformDataTest, DynamicAndGlobalUniformDataMatchStd140Layout)
{
	static_assert(sizeof(DynamicUniformData) == 128);
	static_assert(offsetof(DynamicUniformData, model) == 0);
	static_assert(offsetof(DynamicUniformData, prevModel) == 64);

	static_assert(sizeof(GlobalUniformData) == 544);
	static_assert(offsetof(GlobalUniformData, viewProj) == 128);
	static_assert(offsetof(GlobalUniformData, invViewProj) == 192);
	static_assert(offsetof(GlobalUniformData, unjitteredViewProj) == 256);
	static_assert(offsetof(GlobalUniformData, prevUnjitteredViewProj) == 320);
	static_assert(offsetof(GlobalUniformData, pointLightCount) == 432);
	static_assert(offsetof(GlobalUniformData, frameIndex) == 436);
	static_assert(offsetof(GlobalUniformData, screenWidth) == 440);
	static_assert(offsetof(GlobalUniformData, screenHeight) == 444);
	static_assert(offsetof(GlobalUniformData, debugView) == 448);
	static_assert(offsetof(GlobalUniformData, toneMapMode) == 452);
	static_assert(offsetof(GlobalUniformData, flags) == 456);
	static_assert(offsetof(GlobalUniformData, taaBlendFactor) == 460);
	static_assert(offsetof(GlobalUniformData, skyLuminanceNits) == 464);
	static_assert(offsetof(GlobalUniformData, time) == 496);
	static_assert(offsetof(GlobalUniformData, cloudShadowStrength) == 540);

	SUCCEED();
}
