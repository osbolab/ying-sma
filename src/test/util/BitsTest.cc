#include <iostream>
#include <bitset>
#include "gtest/gtest.h"

#include "log.hh"

#include "bits.hh"


TEST(Builtins, AssertionTrue)
{
#ifdef _DEBUG
  int l = 8;
  std::stringstream ss;
  ss << "ms " << "fedcba9876543210" << " ls" << std::endl;
  std::cout << ss.str();
  for (unsigned int k = 1; k < (unsigned short) -1; k *= 2) {
    unsigned int j = k | (1 << (l++ % 16));
    std::cout << std::hex
              << " " << most_set_bit(j) << " "
              << std::bitset<16>(j)
              << " " << least_set_bit(j) << std::endl;
    if (k == 0x80) std::cout << ss.str();
  }
  std::cout << ss.str();
#endif

  unsigned int v = 1;
  for (int i = 0; i < 32; ++i) {
    ASSERT_EQ(i, least_set_bit(v));
    v <<= 1;
  }

  v = -1;
  for (int i = 31; i >=0; --i) {
    ASSERT_EQ(i, most_set_bit(v));
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

  v = 0;
  for (int i = 0; i < 32; ++i) {
    set_bit(i, v);
    ASSERT_EQ(i, most_set_bit(v));
    ASSERT_EQ(i, least_set_bit(v));
    clear_bit(i, v);
  }
}