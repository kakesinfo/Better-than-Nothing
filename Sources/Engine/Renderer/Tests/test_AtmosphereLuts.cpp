#include <gtest/gtest.h>

#include <Renderer/Renderer.hpp>

using namespace BetterThanNothing;

TEST(AtmosphereLutsTest, SpecsMatchHillaireAndIblRequirements)
{
	const auto transmittance = Renderer::GetTransmittanceLutSpec();
	EXPECT_EQ(transmittance.width, 256u);
	EXPECT_EQ(transmittance.height, 64u);
	EXPECT_EQ(transmittance.format, TextureFormat::R16G16B16A16_SFLOAT);

	const auto multiScat = Renderer::GetMultiScatteringLutSpec();
	EXPECT_EQ(multiScat.width, 32u);
	EXPECT_EQ(multiScat.height, 32u);
	EXPECT_EQ(multiScat.format, TextureFormat::R16G16B16A16_SFLOAT);

	const auto skyView = Renderer::GetSkyViewLutSpec();
	EXPECT_EQ(skyView.width, 192u);
	EXPECT_EQ(skyView.height, 108u);
	EXPECT_EQ(skyView.format, TextureFormat::R16G16B16A16_SFLOAT);

	const auto irradiance = Renderer::GetIrradianceLutSpec();
	EXPECT_EQ(irradiance.width, 64u);
	EXPECT_EQ(irradiance.height, 32u);
	EXPECT_EQ(irradiance.format, TextureFormat::R16G16B16A16_SFLOAT);

	const auto prefiltered = Renderer::GetPrefilteredEnvLutSpec();
	EXPECT_EQ(prefiltered.width, 128u);
	EXPECT_EQ(prefiltered.height, 320u);
	EXPECT_EQ(prefiltered.format, TextureFormat::R16G16B16A16_SFLOAT);

	const auto brdf = Renderer::GetBrdfLutSpec();
	EXPECT_EQ(brdf.width, 256u);
	EXPECT_EQ(brdf.height, 256u);
	EXPECT_EQ(brdf.format, TextureFormat::R16G16_SFLOAT);
}

TEST(AtmosphereLutsTest, SkyViewSamplerRepeatsInUAndClampsInV)
{
	const SamplerDescription desc = Renderer::MakeSkyViewSamplerDesc();
	EXPECT_EQ(desc.magFilter, SamplerFilter::Linear);
	EXPECT_EQ(desc.minFilter, SamplerFilter::Linear);
	EXPECT_EQ(desc.addressModeU, SamplerAddressMode::Repeat);
	EXPECT_EQ(desc.addressModeV, SamplerAddressMode::ClampToEdge);
	EXPECT_EQ(desc.addressModeW, SamplerAddressMode::ClampToEdge);
	EXPECT_FALSE(desc.anisotropyEnable);
}
