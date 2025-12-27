/*
 *	Better than Nothing
 *	Copyright (C) 2025 kakesinfo
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 * 	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "Module_Math/Quaternion.hpp"
#include "Module_Math/Vector3.hpp"
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(Quaternion, Create_Default)
{
	const Quaternion quat;

	EXPECT_FLOAT_EQ(quat.GetW(), 0.0f);
	EXPECT_FLOAT_EQ(quat.GetX(), 0.0f);
	EXPECT_FLOAT_EQ(quat.GetY(), 0.0f);
	EXPECT_FLOAT_EQ(quat.GetZ(), 0.0f);
}

TEST(Quaternion, Create_With_Values)
{
	const Quaternion quat(1.0f, 2.0f, 3.0f, 4.0f);

	EXPECT_FLOAT_EQ(quat.GetW(), 1.0f);
	EXPECT_FLOAT_EQ(quat.GetX(), 2.0f);
	EXPECT_FLOAT_EQ(quat.GetY(), 3.0f);
	EXPECT_FLOAT_EQ(quat.GetZ(), 4.0f);
}

TEST(Quaternion, Create_With_GLM_Quat)
{
	const Quaternion quat(glm::quat(5.0f, 6.0f, 7.0f, 8.0f));

	EXPECT_FLOAT_EQ(quat.GetW(), 5.0f);
	EXPECT_FLOAT_EQ(quat.GetX(), 6.0f);
	EXPECT_FLOAT_EQ(quat.GetY(), 7.0f);
	EXPECT_FLOAT_EQ(quat.GetZ(), 8.0f);
}

TEST(Quaternion, Addition_Operator)
{
	const Quaternion quat1(1.0f, 2.0f, 3.0f, 4.0f);
	const Quaternion quat2(5.0f, 6.0f, 7.0f, 8.0f);

	const Quaternion result = quat1 + quat2;

	EXPECT_FLOAT_EQ(result.GetW(), 6.0f);
	EXPECT_FLOAT_EQ(result.GetX(), 8.0f);
	EXPECT_FLOAT_EQ(result.GetY(), 10.0f);
	EXPECT_FLOAT_EQ(result.GetZ(), 12.0f);
}

TEST(Quaternion, Subtraction_Operator)
{
	const Quaternion quat1(9.0f, 8.0f, 7.0f, 6.0f);
	const Quaternion quat2(5.0f, 4.0f, 3.0f, 2.0f);

	const Quaternion result = quat1 - quat2;

	EXPECT_FLOAT_EQ(result.GetW(), 4.0f);
	EXPECT_FLOAT_EQ(result.GetX(), 4.0f);
	EXPECT_FLOAT_EQ(result.GetY(), 4.0f);
	EXPECT_FLOAT_EQ(result.GetZ(), 4.0f);
}

TEST(Quaternion, Multiplication_Operator)
{
	const Quaternion quat1(1.0f, 0.0f, 1.0f, 0.0f);
	const Quaternion quat2(1.0f, 0.5f, 0.5f, 0.75f);

	const Quaternion result = quat1 * quat2;

	EXPECT_FLOAT_EQ(result.GetW(), 0.5f);
	EXPECT_FLOAT_EQ(result.GetX(), 1.25f);
	EXPECT_FLOAT_EQ(result.GetY(), 1.5f);
	EXPECT_FLOAT_EQ(result.GetZ(), 0.25f);
}

// TODO: Implement vector multiplication tests

TEST(Quaternion, AngleAxis_Method)
{
	const Quaternion quat(1.0f, 0.0f, 0.0f, 0.0f);
	constexpr float	 angle = 90.0f;
	const Vector3	 axis(0.0f, 1.0f, 0.0f);

	const Quaternion result = quat.AngleAxis(angle, axis);

	EXPECT_NEAR(result.GetW(), std::cos(glm::radians(angle) / 2.0f), 1e-5f);
	EXPECT_NEAR(result.GetX(), 0.0f, 1e-5f);
	EXPECT_NEAR(result.GetY(), std::sin(glm::radians(angle) / 2.0f), 1e-5f);
	EXPECT_NEAR(result.GetZ(), 0.0f, 1e-5f);
}

TEST(Quaternion, Normalized_Method)
{
	const Quaternion quat(0.0f, 3.0f, 4.0f, 0.0f);

	const Quaternion normalizedQuat = quat.Normalized();

	const float magnitude = std::sqrt(0.0f * 0.0f + 3.0f * 3.0f + 4.0f * 4.0f + 0.0f * 0.0f);

	EXPECT_FLOAT_EQ(normalizedQuat.GetW(), 0.0f / magnitude);
	EXPECT_FLOAT_EQ(normalizedQuat.GetX(), 3.0f / magnitude);
	EXPECT_FLOAT_EQ(normalizedQuat.GetY(), 4.0f / magnitude);
	EXPECT_FLOAT_EQ(normalizedQuat.GetZ(), 0.0f / magnitude);
}

TEST(Quaternion, Equals_Method)
{
	const Quaternion quat1(1.0f, 2.0f, 3.0f, 4.0f);
	const Quaternion quat2(1.0f, 2.0f, 3.0f, 4.0f);
	const Quaternion quat3(5.0f, 6.0f, 7.0f, 8.0f);

	EXPECT_TRUE(quat1.Equals(quat2));
	EXPECT_FALSE(quat1.Equals(quat3));
}