#include "BufferPool.hh"

#include "gtest/gtest.h"

#include <cstdint>
#include <string>


namespace sma
{

TEST(BufferPool, fill_shrink_and_read)
{
  auto pool = BufferPool<std::uint8_t, 16>(128);
  auto buf = pool.allocate(50);

  std::size_t n = 15;
  unsigned char* arr = new unsigned char[n];
  memset(arr, 'A', n);

  buf.fill_with(arr, n);

  ASSERT_EQ(16, buf.shrink_to_fit());

  for (unsigned int i = 0; i < n; ++i) {
    buf[i] = 'B';
  }

  ASSERT_EQ(n, buf.read_into(arr, n));

  delete[] arr;
}
}
