#include <windows.h>

#include <gtest/gtest.h>

#include <clarinoid/core/basic/basic.hpp>
#include <clarinoid/core/basic/geometry/point.hpp>

namespace Point
{


// Point tests for integer type
TEST(PointTest, DefaultConstructor)
{
  clarinoid::Point<int> p;
  EXPECT_EQ(0, p.x);
  EXPECT_EQ(0, p.y);
}

TEST(PointTest, ParameterizedConstructor)
{
  clarinoid::Point<int> p(3, 4);
  EXPECT_EQ(3, p.x);
  EXPECT_EQ(4, p.y);
}

TEST(PointTest, StaticConstruct)
{
  auto p = clarinoid::Point<int>::Construct(5, 7);
  EXPECT_EQ(5, p.x);
  EXPECT_EQ(7, p.y);
}

TEST(PointTest, WithX)
{
  clarinoid::Point<int> p(3, 4);
  auto p2 = p.WithX(10);
  EXPECT_EQ(10, p2.x);
  EXPECT_EQ(4, p2.y);

  // Original point should be unchanged
  EXPECT_EQ(3, p.x);
  EXPECT_EQ(4, p.y);
}

TEST(PointTest, WithY)
{
  clarinoid::Point<int> p(3, 4);
  auto p2 = p.WithY(10);
  EXPECT_EQ(3, p2.x);
  EXPECT_EQ(10, p2.y);

  // Original point should be unchanged
  EXPECT_EQ(3, p.x);
  EXPECT_EQ(4, p.y);
}

TEST(PointTest, WithXOffset)
{
  clarinoid::Point<int> p(3, 4);
  auto p2 = p.WithXOffset(2);
  EXPECT_EQ(5, p2.x);
  EXPECT_EQ(4, p2.y);

  auto p3 = p.WithXOffset(-1);
  EXPECT_EQ(2, p3.x);
  EXPECT_EQ(4, p3.y);
}

TEST(PointTest, WithYOffset)
{
  clarinoid::Point<int> p(3, 4);
  auto p2 = p.WithYOffset(2);
  EXPECT_EQ(3, p2.x);
  EXPECT_EQ(6, p2.y);

  auto p3 = p.WithYOffset(-3);
  EXPECT_EQ(3, p3.x);
  EXPECT_EQ(1, p3.y);
}

TEST(PointTest, WithOffsetValues)
{
  clarinoid::Point<int> p(3, 4);
  auto p2 = p.WithOffset(2, 1);
  EXPECT_EQ(5, p2.x);
  EXPECT_EQ(5, p2.y);

  auto p3 = p.WithOffset(-2, -1);
  EXPECT_EQ(1, p3.x);
  EXPECT_EQ(3, p3.y);
}

TEST(PointTest, WithOffsetPoint)
{
  clarinoid::Point<int> p1(3, 4);
  clarinoid::Point<int> offset(2, 1);
  auto p2 = p1.WithOffset(offset);
  EXPECT_EQ(5, p2.x);
  EXPECT_EQ(5, p2.y);

  clarinoid::Point<int> negativeOffset(-1, -2);
  auto p3 = p1.WithOffset(negativeOffset);
  EXPECT_EQ(2, p3.x);
  EXPECT_EQ(2, p3.y);
}

TEST(PointTest, Cast)
{
  clarinoid::Point<int> pi(3, 4);
  auto pf = pi.Cast<float>();
  EXPECT_FLOAT_EQ(3.0f, pf.x);
  EXPECT_FLOAT_EQ(4.0f, pf.y);

  clarinoid::Point<double> pd(3.7, 4.2);
  auto pi2 = pd.Cast<int>();
  EXPECT_EQ(3, pi2.x);
  EXPECT_EQ(4, pi2.y);
}

// Point tests for floating point type
TEST(PointFloatTest, DefaultConstructor)
{
  clarinoid::Point<float> p;
  EXPECT_FLOAT_EQ(0.0f, p.x);
  EXPECT_FLOAT_EQ(0.0f, p.y);
}

TEST(PointFloatTest, ParameterizedConstructor)
{
  clarinoid::Point<float> p(3.5f, 4.2f);
  EXPECT_FLOAT_EQ(3.5f, p.x);
  EXPECT_FLOAT_EQ(4.2f, p.y);
}

TEST(PointFloatTest, WithOffsetPrecision)
{
  clarinoid::Point<float> p(1.1f, 2.2f);
  auto p2 = p.WithOffset(0.1f, 0.3f);
  EXPECT_FLOAT_EQ(1.2f, p2.x);
  EXPECT_FLOAT_EQ(2.5f, p2.y);
}

// Test with double precision
TEST(PointDoubleTest, HighPrecision)
{
  clarinoid::Point<double> p(1.23456789, 9.87654321);
  EXPECT_DOUBLE_EQ(1.23456789, p.x);
  EXPECT_DOUBLE_EQ(9.87654321, p.y);

  auto p2 = p.WithOffset(0.00000001, -0.00000001);
  EXPECT_DOUBLE_EQ(1.2345679, p2.x);
  EXPECT_DOUBLE_EQ(9.8765432, p2.y);
}

// Edge cases
TEST(PointEdgeCasesTest, ZeroValues)
{
  clarinoid::Point<int> p(0, 0);

  auto p2 = p.WithOffset(0, 0);
  EXPECT_EQ(0, p2.x);
  EXPECT_EQ(0, p2.y);
}

TEST(PointEdgeCasesTest, NegativeValues)
{
  clarinoid::Point<int> p(-3, -4);
  EXPECT_EQ(-3, p.x);
  EXPECT_EQ(-4, p.y);

  auto p2 = p.WithX(5);
  EXPECT_EQ(5, p2.x);
  EXPECT_EQ(-4, p2.y);
}

}  // namespace Point
