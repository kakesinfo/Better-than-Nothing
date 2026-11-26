#include <Data/Graphics/DynamicUniformData.hpp>
#include <Data/Graphics/GlobalUniformData.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(UniformDataTest, DynamicAndGlobalUniformDataMatchStd140Layout)
{
	static_assert(sizeof(DynamicUniformData) == 128);
	static_assert(offsetof(DynamicUniformData, model) == 0);
	static_assert(offsetof(DynamicUniformData, prevModel) == 64);

	static_assert(sizeof(GlobalUniformData) == 496);
	static_assert(offsetof(GlobalUniformData, viewProj) == 128);
	static_assert(offsetof(GlobalUniformData, invViewProj) == 192);
	static_assert(offsetof(GlobalUniformData, unjitteredViewProj) == 256);
	static_assert(offsetof(GlobalUniformData, prevUnjitteredViewProj) == 320);
	static_assert(offsetof(GlobalUniformData, screenWidth) == 432);
	static_assert(offsetof(GlobalUniformData, screenHeight) == 436);
	static_assert(offsetof(GlobalUniformData, frameIndex) == 440);
	static_assert(offsetof(GlobalUniformData, debugView) == 444);
	static_assert(offsetof(GlobalUniformData, toneMapMode) == 448);
	static_assert(offsetof(GlobalUniformData, flags) == 452);
	static_assert(offsetof(GlobalUniformData, elapsedTime) == 456);
	static_assert(offsetof(GlobalUniformData, deltaTime) == 460);
	static_assert(offsetof(GlobalUniformData, irradianceIntensity) == 464);
	static_assert(offsetof(GlobalUniformData, waterWaveScale) == 468);
	static_assert(offsetof(GlobalUniformData, waterFoamIntensity) == 472);
	static_assert(offsetof(GlobalUniformData, grassWindStrength) == 476);
	static_assert(offsetof(GlobalUniformData, grassWindSpeed) == 480);
	static_assert(offsetof(GlobalUniformData, grassFadeDistance) == 484);
	static_assert(offsetof(GlobalUniformData, cloudDensity) == 488);
	static_assert(offsetof(GlobalUniformData, cloudShadowStrength) == 492);

	SUCCEED();
}
