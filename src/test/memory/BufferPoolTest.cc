#include <iostream>
#include <cstdint>
#include <string>

#include "gtest/gtest.h"

#include "memory/BufferPool.hh"


namespace sma
{

TEST(It_Compiles, AssertionTrue)
{
  auto pool = BufferPool<std::uint8_t, 16>(128);
  auto buf = pool.allocate(50);

  std::size_t n = 15;
  unsigned char* arr = new unsigned char[n];
  memset(arr, 'A', n);

  buf.fill_with(arr, n);
  std::cout << std::string(arr, arr+n) << std::endl;
  std::cout << "capacity: " << buf.capacity() << " size " << buf.size() << std::endl;

  ASSERT_EQ(16, buf.shrink_to_fit());

  std::cout << "capacity: " << buf.capacity() << " size " << buf.size() << std::endl;

  for (unsigned int i = 0; i < n; ++i) {
    buf[i] = 'B';
  }

  ASSERT_EQ(n, buf.read_into(arr, n));
  std::cout << std::string(arr, arr+n) << std::endl;

  delete[] arr;
}

}