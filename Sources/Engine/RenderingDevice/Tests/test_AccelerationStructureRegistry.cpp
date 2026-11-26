#include <RenderingApi/Resources/AccelerationStructure.hpp>
#include <RenderingDevice/Handle.hpp>
#include <RenderingDevice/HandleRegistry.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(AccelerationStructureRegistry, AllocateRetrieveAndDestroy)
{
	HandleRegistry<RenderingApi::AccelerationStructure, HandleType::AccelerationStructure> registry;

	const Handle h = registry.Allocate();
	EXPECT_TRUE(h.IsValid());
	EXPECT_EQ(h.GetType(), static_cast<uint8_t>(HandleType::AccelerationStructure));
	EXPECT_TRUE(registry.Has(h));

	auto* as = registry.Get(h);
	ASSERT_NE(as, nullptr);
	as->deviceAddress = 0x12345678ULL;
	as->size = 4096;

	EXPECT_EQ(registry.Get(h)->deviceAddress, 0x12345678ULL);
	EXPECT_EQ(registry.Get(h)->size, 4096u);

	registry.Destroy(h);
	EXPECT_FALSE(registry.Has(h));
}
