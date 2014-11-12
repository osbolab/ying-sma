#include <sma/core/message.hpp>
#include <sma/core/bytes.hpp>

#include "gtest/gtest.h"

#include <string>
#include <cstdint>


namespace sma
{

TEST(message, serialize_deserialize)
{
  std::string contents{"Hello, world!"};
  auto bytes = uint8_cp(contents);
  auto msg
      = message(message::build(message::Type{1}, bytes, contents.size()),
                message::Address{1});

  std::uint8_t buf[32];
  msg.put_in(buf, sizeof buf);

  auto msg2 = message(buf, sizeof buf);

  ASSERT_EQ(1, msg2.type());
  ASSERT_EQ(contents, copy_string(msg2.body(), msg2.body_size()));
}
}
