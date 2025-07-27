

#include <gtest/gtest.h>
#include <desktop/ArduinoDesktop.hpp>
#include <clarinoid/basic/BaseDefs.hpp>

int
main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(BaseTests, CopyPodArrayArgOrderLTR)
{
  int x[3] = { 1, 2, 3 };
  int y[3] = {};
  clarinoid::CopyPODArray(x, y);
  EXPECT_EQ(y[0], 1);
  EXPECT_EQ(y[1], 2);
  EXPECT_EQ(y[2], 3);
}