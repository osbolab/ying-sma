#include <iostream>

#include "gtest/gtest.h"

#include "memory/BufferPool.hh"
#include "memory/pool_ptr.hh"


TEST(It_Compiles, AssertionTrue)
{
  auto pool = sma::BufferPool::create(128);
  auto buf = pool->allocate(3);
}