#include "message.hh"
#include "bytes.hh"

#include "gtest/gtest.h"

#include <string>
#include <cstdint>


namespace sma
{

TEST(Message, serialize_deserialize)
{
  std::string contents{"Hello, world!"};
  auto bytes = uint8_cp(contents);
  auto msg
      = Message(Message::build(Message::Type{1}, bytes, contents.size()),
                Message::Address{1});

  std::uint8_t buf[32];
  msg.put_in(buf, sizeof buf);

  auto msg2 = Message(buf, sizeof buf);

  ASSERT_EQ(1, msg2.type());
  ASSERT_EQ(contents, copy_string(msg2.body(), msg2.body_size()));
}
}
