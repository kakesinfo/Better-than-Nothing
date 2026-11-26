#include <Scene/SceneObject_Components.hpp>
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(TemporalSnapshotTest, PrevWorldMatrixAndPingPongToggleUpdateProperly)
{
	TransformComponent transform{};
	transform.worldMatrix = Matrix4::Identity.Translate(1.0f, 2.0f, 3.0f);
	EXPECT_EQ(transform.prevWorldMatrix, Matrix4::Identity);

	transform.prevWorldMatrix = transform.worldMatrix;
	EXPECT_EQ(transform.prevWorldMatrix, transform.worldMatrix);

	uint32_t temporalReadIndex = 0u;
	temporalReadIndex = 1u - temporalReadIndex;
	EXPECT_EQ(temporalReadIndex, 1u);
	temporalReadIndex = 1u - temporalReadIndex;
	EXPECT_EQ(temporalReadIndex, 0u);
}
