#include <iostream>

#include "gtest/gtest.h"

#include "memory/BufferPool.hh"

TEST(It_Compiles, AssertionTrue)
{
  auto pool = sma::BufferPool::allocate(128);
  auto buf = pool.get(3);
}