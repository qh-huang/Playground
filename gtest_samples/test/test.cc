#include <gtest_samples/gtest_samples_demo.h>
#include <gtest/gtest.h>

using namespace gtest_samples_demo;

TEST(example, SumOfThreeIntegers) {
  const int a = 1;
  const int b = 2;
  const int c = 3;
  const int res = SumOfThreeIntegers(a, b, c);
  EXPECT_EQ(res, 6);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}