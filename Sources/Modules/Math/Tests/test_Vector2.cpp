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

#include "Module_Math/Vector2.hpp"
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(Vector2, Create_Default)
{
	const Vector2 vec;

	EXPECT_FLOAT_EQ(vec.GetX(), 0.0f);
	EXPECT_FLOAT_EQ(vec.GetY(), 0.0f);
}

TEST(Vector2, Create_WithValues)
{
	const Vector2 vec(3.0f, 4.0f);

	EXPECT_FLOAT_EQ(vec.GetX(), 3.0f);
	EXPECT_FLOAT_EQ(vec.GetY(), 4.0f);
}

TEST(Vector2, Create_From_glm_vec2)
{
	const Vector2 vec(glm::vec2(5.0f, 6.0f));

	EXPECT_FLOAT_EQ(vec.GetX(), 5.0f);
	EXPECT_FLOAT_EQ(vec.GetY(), 6.0f);
}

TEST(Vector2, Addition_Operator)
{
	const Vector2 vec1(1.0f, 2.0f);
	const Vector2 vec2(3.0f, 4.0f);

	const Vector2 result = vec1 + vec2;

	EXPECT_FLOAT_EQ(result.GetX(), 4.0f);
	EXPECT_FLOAT_EQ(result.GetY(), 6.0f);
}

TEST(Vector2, Subtraction_Operator)
{
	const Vector2 vec1(5.0f, 7.0f);
	const Vector2 vec2(2.0f, 3.0f);

	const Vector2 result = vec1 - vec2;

	EXPECT_FLOAT_EQ(result.GetX(), 3.0f);
	EXPECT_FLOAT_EQ(result.GetY(), 4.0f);
}

TEST(Vector2, Multiplication_Operator)
{
	const Vector2	vec(2.0f, 3.0f);
	constexpr float scalar = 4.0f;

	const Vector2 result = vec * scalar;

	EXPECT_FLOAT_EQ(result.GetX(), 8.0f);
	EXPECT_FLOAT_EQ(result.GetY(), 12.0f);
}

TEST(Vector2, Division_Operator)
{
	const Vector2	vec(8.0f, 12.0f);
	constexpr float scalar = 4.0f;

	const Vector2 result = vec / scalar;

	EXPECT_FLOAT_EQ(result.GetX(), 2.0f);
	EXPECT_FLOAT_EQ(result.GetY(), 3.0f);
}

TEST(Vector2, Equals_Method)
{
	const Vector2 vec1(1.0f, 2.0f);
	const Vector2 vec2(1.0f, 2.0f);
	const Vector2 vec3(2.0f, 3.0f);

	EXPECT_TRUE(vec1.Equals(vec2));
	EXPECT_FALSE(vec1.Equals(vec3));
}

TEST(Vector2, Lerp_Method)
{
	const Vector2	start(0.0f, 0.0f);
	const Vector2	end(10.0f, 10.0f);
	constexpr float t = 0.5f;

	const Vector2 result = Vector2::Lerp(start, end, t);

	EXPECT_FLOAT_EQ(result.GetX(), 5.0f);
	EXPECT_FLOAT_EQ(result.GetY(), 5.0f);
}

TEST(Vector2, Min_Method)
{
	const Vector2 vec1(1.0f, 4.0f);
	const Vector2 vec2(3.0f, 2.0f);

	const Vector2 result = Vector2::Min(vec1, vec2);

	EXPECT_FLOAT_EQ(result.GetX(), 1.0f);
	EXPECT_FLOAT_EQ(result.GetY(), 2.0f);
}

TEST(Vector2, Max_Method)
{
	const Vector2 vec1(1.0f, 4.0f);
	const Vector2 vec2(3.0f, 2.0f);

	const Vector2 result = Vector2::Max(vec1, vec2);

	EXPECT_FLOAT_EQ(result.GetX(), 3.0f);
	EXPECT_FLOAT_EQ(result.GetY(), 4.0f);
}

TEST(Vector2, Magnitude)
{
	const Vector2 vec(3.0f, 4.0f);

	const float magnitude = vec.Magnitude();

	EXPECT_FLOAT_EQ(magnitude, 5.0f);
}

TEST(Vector2, Normalized)
{
	const Vector2 vec(3.0f, 4.0f);

	const Vector2 normalizedVec = vec.Normalized();

	EXPECT_FLOAT_EQ(normalizedVec.GetX(), 0.6f);
	EXPECT_FLOAT_EQ(normalizedVec.GetY(), 0.8f);
}

TEST(Vector2, Dot_Product)
{
	const Vector2 vec1(1.0f, 2.0f);
	const Vector2 vec2(3.0f, 4.0f);

	const float dotProduct = Vector2::Dot(vec1, vec2);

	EXPECT_FLOAT_EQ(dotProduct, 11.0f);
}

TEST(Vector2, Distance)
{
	const Vector2 vec1(1.0f, 2.0f);
	const Vector2 vec2(4.0f, 6.0f);

	const float distance = Vector2::Distance(vec1, vec2);

	EXPECT_FLOAT_EQ(distance, 5.0f);
}
