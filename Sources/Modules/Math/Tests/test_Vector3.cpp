/*
 *	Better than Nothing
 *	Copyright (C) 2026 kakesinfo
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

#include "Module_Math/Vector3.hpp"
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(Vector3, Create_Default)
{
	const Vector3 vec;

	EXPECT_FLOAT_EQ(vec.GetX(), 0.0f);
	EXPECT_FLOAT_EQ(vec.GetY(), 0.0f);
	EXPECT_FLOAT_EQ(vec.GetZ(), 0.0f);
}

TEST(Vector3, Create_WithValues)
{
	const Vector3 vec(3.0f, 4.0f, 5.0f);

	EXPECT_FLOAT_EQ(vec.GetX(), 3.0f);
	EXPECT_FLOAT_EQ(vec.GetY(), 4.0f);
	EXPECT_FLOAT_EQ(vec.GetZ(), 5.0f);
}

TEST(Vector3, Create_From_glm_vec3)
{
	const Vector3 vec(glm::vec3(5.0f, 6.0f, 7.0f));

	EXPECT_FLOAT_EQ(vec.GetX(), 5.0f);
	EXPECT_FLOAT_EQ(vec.GetY(), 6.0f);
	EXPECT_FLOAT_EQ(vec.GetZ(), 7.0f);
}

TEST(Vector3, Addition_Operator)
{
	const Vector3 vec1(1.0f, 2.0f, 3.0f);
	const Vector3 vec2(3.0f, 4.0f, 5.0f);

	const Vector3 result = vec1 + vec2;

	EXPECT_FLOAT_EQ(result.GetX(), 4.0f);
	EXPECT_FLOAT_EQ(result.GetY(), 6.0f);
	EXPECT_FLOAT_EQ(result.GetZ(), 8.0f);
}

TEST(Vector3, Subtraction_Operator)
{
	const Vector3 vec1(5.0f, 7.0f, 9.0f);
	const Vector3 vec2(2.0f, 3.0f, 4.0f);

	const Vector3 result = vec1 - vec2;

	EXPECT_FLOAT_EQ(result.GetX(), 3.0f);
	EXPECT_FLOAT_EQ(result.GetY(), 4.0f);
	EXPECT_FLOAT_EQ(result.GetZ(), 5.0f);
}

TEST(Vector3, Multiplication_Operator)
{
	const Vector3	vec(2.0f, 3.0f, 4.0f);
	constexpr float scalar = 4.0f;

	const Vector3 result = vec * scalar;

	EXPECT_FLOAT_EQ(result.GetX(), 8.0f);
	EXPECT_FLOAT_EQ(result.GetY(), 12.0f);
	EXPECT_FLOAT_EQ(result.GetZ(), 16.0f);
}

TEST(Vector3, Division_Operator)
{
	const Vector3	vec(8.0f, 12.0f, 16.0f);
	constexpr float scalar = 4.0f;

	const Vector3 result = vec / scalar;

	EXPECT_FLOAT_EQ(result.GetX(), 2.0f);
	EXPECT_FLOAT_EQ(result.GetY(), 3.0f);
	EXPECT_FLOAT_EQ(result.GetZ(), 4.0f);
}

TEST(Vector3, Equals_Method)
{
	const Vector3 vec1(1.0f, 2.0f, 3.0f);
	const Vector3 vec2(1.0f, 2.0f, 3.0f);
	const Vector3 vec3(4.0f, 5.0f, 6.0f);

	EXPECT_TRUE(vec1.Equals(vec2));
	EXPECT_FALSE(vec1.Equals(vec3));
}

TEST(Vector3, Lerp_Method)
{
	const Vector3	start(0.0f, 0.0f, 0.0f);
	const Vector3	end(10.0f, 10.0f, 10.0f);
	constexpr float t = 0.5f;

	const Vector3 result = Vector3::Lerp(start, end, t);

	EXPECT_FLOAT_EQ(result.GetX(), 5.0f);
	EXPECT_FLOAT_EQ(result.GetY(), 5.0f);
	EXPECT_FLOAT_EQ(result.GetZ(), 5.0f);
}

TEST(Vector3, Min_Method)
{
	const Vector3 vec1(1.0f, 4.0f, 6.0f);
	const Vector3 vec2(3.0f, 2.0f, 5.0f);

	const Vector3 result = Vector3::Min(vec1, vec2);

	EXPECT_FLOAT_EQ(result.GetX(), 1.0f);
	EXPECT_FLOAT_EQ(result.GetY(), 2.0f);
	EXPECT_FLOAT_EQ(result.GetZ(), 5.0f);
}

TEST(Vector3, Max_Method)
{
	const Vector3 vec1(1.0f, 4.0f, 6.0f);
	const Vector3 vec2(3.0f, 2.0f, 5.0f);

	const Vector3 result = Vector3::Max(vec1, vec2);

	EXPECT_FLOAT_EQ(result.GetX(), 3.0f);
	EXPECT_FLOAT_EQ(result.GetY(), 4.0f);
	EXPECT_FLOAT_EQ(result.GetZ(), 6.0f);
}

TEST(Vector3, Magnitude)
{
	const Vector3 vec(3.0f, 4.0f, 12.0f);

	const float magnitude = vec.Magnitude();

	EXPECT_FLOAT_EQ(magnitude, 13.0f);
}

TEST(Vector3, Normalized)
{
	const Vector3 vec(3.0f, 4.0f, 0.0f);

	const Vector3 normalizedVec = vec.Normalized();

	EXPECT_FLOAT_EQ(normalizedVec.GetX(), 0.6f);
	EXPECT_FLOAT_EQ(normalizedVec.GetY(), 0.8f);
	EXPECT_FLOAT_EQ(normalizedVec.GetZ(), 0.0f);
}

TEST(Vector3, Dot_Product)
{
	const Vector3 vec1(1.0f, 2.0f, 3.0f);
	const Vector3 vec2(4.0f, 5.0f, 6.0f);

	const float dotProduct = Vector3::Dot(vec1, vec2);

	EXPECT_FLOAT_EQ(dotProduct, 32.0f);
}

TEST(Vector3, Distance)
{
	const Vector3 vec1(1.0f, 2.0f, 3.0f);
	const Vector3 vec2(4.0f, 6.0f, 3.0f);

	const float distance = Vector3::Distance(vec1, vec2);

	EXPECT_FLOAT_EQ(distance, 5.0f);
}
