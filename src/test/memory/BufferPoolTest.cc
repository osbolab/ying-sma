#include <iostream>

#include "gtest/gtest.h"

#include "memory/BufferPool.hh"

TEST(It_Compiles, AssertionTrue)
{
  auto pool = sma::BufferPool<2>::create(6);
}