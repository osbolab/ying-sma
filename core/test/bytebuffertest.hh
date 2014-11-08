#pragma once

#include "bytebuffer.hh"

#include "gtest/gtest.h"

#include <cstdint>


namespace sma
{

TEST(ByteBuffer, write_and_read)
{
  char cbuf[32];

  auto u32 = std::uint32_t{1792362};
  auto u16 = std::uint16_t{16932};
  auto u8 = std::uint8_t{172};

  auto buf = ByteBuffer::wrap(cbuf, sizeof cbuf);
  buf << u32 << u16 << u8;
  buf.flip();

  ASSERT_EQ(u32, buf.get<std::uint32_t>());
  ASSERT_EQ(u16, buf.get<std::uint16_t>());
  ASSERT_EQ(u8, buf.get<std::uint8_t>());
  buf.flip();

  auto buf2 = ByteBuffer::allocate(sizeof cbuf);
  buf2 << u32 << u16 << u8;
  buf2.flip();
  ASSERT_EQ(buf, buf2);

  auto buf3 = buf2.duplicate();
  ASSERT_EQ(buf2, buf3);

  auto v1 = buf2.view();
  auto cv1 = buf3.cview();
  ASSERT_EQ(v1, cv1);

  buf2.clear();
  buf3.clear();
  buf2 << u16 << u32 << u16 << u8;
  buf3 << u16 << u32 << u16 << u8;
  v1.flip();
  cv1.flip();

  ASSERT_EQ(v1, cv1);
}
}
