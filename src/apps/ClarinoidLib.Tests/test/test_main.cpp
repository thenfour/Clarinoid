#include <windows.h>

#include <gtest/gtest.h>

#include <lib_example/example.hpp>

TEST(BasicTest, SanityCheck)
{
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);
}

TEST(TestPointTest, Increment)
{
  extern int gTestPoint;
  int initial = gTestPoint;
  gTestPoint++;
  EXPECT_EQ(gTestPoint, initial + 1);
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}