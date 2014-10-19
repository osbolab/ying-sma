#include <iostream>
#include <cstdint>
#include <string>

#include "gtest/gtest.h"

#include "memory/buffer_pool.hh"
#include "memory/pool_buf.hh"

TEST(It_Compiles, AssertionTrue)
{
  auto pool = sma::buffer_pool<std::uint8_t, 32>(128);
  auto pool2 = std::move(pool);
  pool = std::move(pool2);
  auto buf = pool.allocate(50);
  auto buf2 = std::move(buf);
  buf = std::move(buf2);

  unsigned char arr[50];
  memset(arr, 'A', 50);

  buf.fill_from(arr, 50);
  std::cout << std::string(arr, arr+50) << std::endl;


  for (int i = 0; i < 65; ++i) {
    buf[i] = 'B';
  }

  ASSERT_EQ(buf.read_into(arr, 50), 50);
  std::cout << std::string(arr, arr+50) << std::endl;
}