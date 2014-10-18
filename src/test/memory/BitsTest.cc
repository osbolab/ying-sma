#include <iostream>
#include "gtest/gtest.h"

#include "memory/bits.hh"

TEST(Builtins, AssertionTrue)
{
  unsigned int v = 1;
  for (int i = 0; i < 32; ++i) {
    ASSERT_EQ(i, ls_bit(v));
    v <<= 1;
  }

  v = -1;
  for (int i = 31; i >=0; --i) {
    ASSERT_EQ(i, ms_bit(v));
    v >>= 1;
  }

  v = 0;
  set_bit(1, v);
  set_bit(2, v);
  ASSERT_EQ(6, v);

  clear_bit(1, v);
  clear_bit(2, v);
  set_bit(0, v);
  set_bit(3, v);
  ASSERT_EQ(9, v);

  int nums[2] { 0, 0 };
  ASSERT_EQ(0, nums[0]);
  ASSERT_EQ(0, nums[1]);

  set_bit_a(32, nums);
  ASSERT_EQ(0, nums[0]);
  ASSERT_EQ(1, nums[1]);

  clear_bit_a(32, nums);
  set_bit_a(31, nums);
  set_bit_a(63, nums);
  ASSERT_EQ(2147483648, nums[0]);
  ASSERT_EQ(2147483648, nums[1]);

  v = 0;
  for (int i = 1; i < 32; ++i) {
    set_bit(i, v);
    ASSERT_EQ(i, ms_bit(v));
    ASSERT_EQ(i, ls_bit(v));
  }
}