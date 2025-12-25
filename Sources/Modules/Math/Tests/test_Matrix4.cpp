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

#include "Module_Math/Matrix4.hpp"
#include "Module_Math/Vector3.hpp"
#include <gtest/gtest.h>

using namespace BetterThanNothing;

TEST(Matrix4, Create_Default)
{
	const Matrix4 mat;

	EXPECT_TRUE(mat.Value() == Matrix4::Identity.Value());
}

TEST(Matrix4, Create_With_Values)
{
	const Matrix4 mat(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	EXPECT_TRUE(mat.Value() == Matrix4::Identity.Value());
}

TEST(Matrix4, Create_With_GLM_Mat4)
{
	constexpr glm::mat4 glmMat(1.0f);
	const Matrix4	mat(glmMat);

	EXPECT_TRUE(mat.Value() == Matrix4::Identity.Value());
}

TEST(Matrix4, Multiplication_By_Scalar_Operator)
{
	const Matrix4 mat(1.0f, 2.0f, 3.0f, 4.0f,
					  5.0f, 6.0f, 7.0f, 8.0f,
					  9.0f, 10.0f, 11.0f, 12.0f,
					  13.0f, 14.0f, 15.0f, 16.0f);

	constexpr float scalar = 2.0f;

	const Matrix4 result = mat * scalar;

	const Matrix4 expected(2.0f, 4.0f, 6.0f, 8.0f,
						   10.0f, 12.0f, 14.0f, 16.0f,
						   18.0f, 20.0f, 22.0f, 24.0f,
						   26.0f, 28.0f, 30.0f, 32.0f);

	EXPECT_TRUE(result.Value() == expected.Value());
}

TEST(Matrix4, Multiplication_Operator)
{
	const Matrix4 mat1(1.0f, 0.0f, 0.0f, 0.0f,
					   0.0f, 1.0f, 0.0f, 0.0f,
					   0.0f, 0.0f, 1.0f, 0.0f,
					   1.0f, 2.0f, 3.0f, 1.0f);

	const Matrix4 mat2(1.0f, 0.0f, 0.0f, 0.0f,
					   0.0f, 1.0f, 0.0f, 0.0f,
					   0.0f, 0.0f, 1.0f, 0.0f,
					   4.0f, 5.0f, 6.0f, 1.0f);

	const Matrix4 result = mat1 * mat2;

	const Matrix4 expected(1.0f, 0.0f, 0.0f, 0.0f,
						   0.0f, 1.0f, 0.0f, 0.0f,
						   0.0f, 0.0f, 1.0f, 0.0f,
						   5.0f, 7.0f, 9.0f, 1.0f);

	EXPECT_TRUE(result.Value() == expected.Value());
}

TEST(Matrix4, Addition_Operator)
{
	const Matrix4 mat1(1.0f, 2.0f, 3.0f, 4.0f,
					   5.0f, 6.0f, 7.0f, 8.0f,
					   9.0f, 10.0f, 11.0f, 12.0f,
					   13.0f, 14.0f, 15.0f, 16.0f);

	const Matrix4 mat2(16.0f, 15.0f, 14.0f, 13.0f,
					   12.0f, 11.0f, 10.0f, 9.0f,
					   8.0f, 7.0f, 6.0f, 5.0f,
					   4.0f, 3.0f, 2.0f, 1.0f);

	const Matrix4 result = mat1 + mat2;

	const Matrix4 expected(17.0f, 17.0f, 17.0f, 17.0f,
						   17.0f, 17.0f, 17.0f, 17.0f,
						   17.0f, 17.0f, 17.0f, 17.0f,
						   17.0f, 17.0f, 17.0f, 17.0f);

	EXPECT_TRUE(result.Value() == expected.Value());
}

TEST(Matrix4, Subtraction_Operator)
{
	const Matrix4 mat1(16.0f, 15.0f, 14.0f, 13.0f,
					   12.0f, 11.0f, 10.0f, 9.0f,
					   8.0f, 7.0f, 6.0f, 5.0f,
					   4.0f, 3.0f, 2.0f, 1.0f);

	const Matrix4 mat2(1.0f, 2.0f, 3.0f, 4.0f,
					   5.0f, 6.0f, 7.0f, 8.0f,
					   9.0f, 10.0f, 11.0f, 12.0f,
					   13.0f, 14.0f, 15.0f, 16.0f);

	const Matrix4 result = mat1 - mat2;

	const Matrix4 expected(15.0f, 13.0f, 11.0f, 9.0f,
						   7.0f, 5.0f, 3.0f, 1.0f,
						   -1.0f, -3.0f, -5.0f, -7.0f,
						   -9.0f, -11.0f, -13.0f, -15.0f);

	EXPECT_TRUE(result.Value() == expected.Value());
}

TEST(Matrix4, Translate_Method)
{
	const Matrix4 mat = Matrix4::Identity;

	const Matrix4 result = mat.Translate(1.0f, 2.0f, 3.0f);

	const Matrix4 expected(1.0f, 0.0f, 0.0f, 1.0f,
						   0.0f, 1.0f, 0.0f, 2.0f,
						   0.0f, 0.0f, 1.0f, 3.0f,
						   0.0f, 0.0f, 0.0f, 1.0f);

	EXPECT_TRUE(result.Value() == expected.Value());
}

TEST(Matrix4, Translate_Vector3_Method)
{
	const Matrix4 mat = Matrix4::Identity;
	const Vector3 vec(1.0f, 2.0f, 3.0f);

	const Matrix4 result = mat.Translate(vec);

	const Matrix4 expected(1.0f, 0.0f, 0.0f, 1.0f,
						   0.0f, 1.0f, 0.0f, 2.0f,
						   0.0f, 0.0f, 1.0f, 3.0f,
						   0.0f, 0.0f, 0.0f, 1.0f);

	EXPECT_TRUE(result.Value() == expected.Value());
}

// TODO: Implement Rotate tests

TEST(Matrix4, Scale_Method)
{
	const Matrix4 mat = Matrix4::Identity;

	const Matrix4 result = mat.Scale(2.0f, 3.0f, 4.0f);

	const Matrix4 expected(2.0f, 0.0f, 0.0f, 0.0f,
						   0.0f, 3.0f, 0.0f, 0.0f,
						   0.0f, 0.0f, 4.0f, 0.0f,
						   0.0f, 0.0f, 0.0f, 1.0f);

	EXPECT_TRUE(result.Value() == expected.Value());
}

TEST(Matrix4, Scale_Vector3_Method)
{
	const Matrix4 mat = Matrix4::Identity;
	const Vector3 vec(2.0f, 3.0f, 4.0f);

	const Matrix4 result = mat.Scale(vec);

	const Matrix4 expected(2.0f, 0.0f, 0.0f, 0.0f,
						   0.0f, 3.0f, 0.0f, 0.0f,
						   0.0f, 0.0f, 4.0f, 0.0f,
						   0.0f, 0.0f, 0.0f, 1.0f);

	EXPECT_TRUE(result.Value() == expected.Value());
}

TEST(Matrix4, Transposed_Method)
{
	const Matrix4 mat(1.0f, 2.0f, 3.0f, 4.0f,
					  5.0f, 6.0f, 7.0f, 8.0f,
					  9.0f, 10.0f, 11.0f, 12.0f,
					  13.0f, 14.0f, 15.0f, 16.0f);

	const Matrix4 result = mat.Transposed();

	const Matrix4 expected(1.0f, 5.0f, 9.0f, 13.0f,
						   2.0f, 6.0f, 10.0f, 14.0f,
						   3.0f, 7.0f, 11.0f, 15.0f,
						   4.0f, 8.0f, 12.0f, 16.0f);

	EXPECT_TRUE(result.Value() == expected.Value());
}

TEST(Matrix4, Inverted_Method)
{
	const Matrix4 mat(1.0f, 0.0f, 0.0f, 0.0f,
					  0.0f, 1.0f, 0.0f, 0.0f,
					  0.0f, 0.0f, 1.0f, 0.0f,
					  1.0f, 2.0f, 3.0f, 1.0f);

	const Matrix4 result = mat.Inverted();

	const Matrix4 expected(1.0f, 0.0f, 0.0f, 0.0f,
						   0.0f, 1.0f, 0.0f, 0.0f,
						   0.0f, 0.0f, 1.0f, 0.0f,
						   -1.0f, -2.0f, -3.0f, 1.0f);

	EXPECT_TRUE(result.Value() == expected.Value());
}

TEST(Matrix4, Equals_Method)
{
	const Matrix4 mat1(1.0f, 2.0f, 3.0f, 4.0f,
					   5.0f, 6.0f, 7.0f, 8.0f,
					   9.0f, 10.0f, 11.0f, 12.0f,
					   13.0f, 14.0f, 15.0f, 16.0f);

	const Matrix4 mat2(1.0f, 2.0f, 3.0f, 4.0f,
					   5.0f, 6.0f, 7.0f, 8.0f,
					   9.0f, 10.0f, 11.0f, 12.0f,
					   13.0f, 14.0f, 15.0f, 16.0f);

	const Matrix4 mat3(16.0f, 15.0f, 14.0f, 13.0f,
					   12.0f, 11.0f, 10.0f, 9.0f,
					   8.0f, 7.0f, 6.0f, 5.0f,
					   4.0f, 3.0f, 2.0f, 1.0f);

	EXPECT_TRUE(mat1.Equals(mat2));
	EXPECT_FALSE(mat1.Equals(mat3));
}

