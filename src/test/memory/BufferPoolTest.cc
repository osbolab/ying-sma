#include <iostream>

#include "gtest/gtest.h"

//#include "memory/BufferPool.hh"
//#include "memory/pool_buf.hh"


#include <cstdint>
#include "test.hh"

TEST(It_Compiles, AssertionTrue)
{
#if 0
  auto pool = sma::BufferPool::create(128);
  auto buf = pool.allocate(50);

  for (std::size_t i = 0; i < 50; ++i) {
    buf[i] = i;
  }

  unsigned char arr[50];
  //buf.readInto(arr, 50);
#endif

  std::size_t szBlock =32;
  int divisor = 0;


  for (std::size_t sz = szBlock; (sz >>= 1) >= 1; ++divisor);

  for (std::size_t index = 0; index < 64; ++index) {
    std::cout << index << " / " << szBlock << " = " << (index >> divisor) << std::endl;
  }


}