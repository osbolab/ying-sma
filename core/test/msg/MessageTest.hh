#include "msg/Message.hh"

#include "gtest/gtest.h"

#include <iostream>
#include <cstdint>
#include <string>
#include <cassert>
#include <memory>
#include <algorithm>



namespace sma
{

std::string contents("Hello, world!");
const char* chars = contents.c_str();

TEST(Message, copy_vector)
{
  auto mut =
      Message(std::vector<std::uint8_t>(contents.begin(), contents.end()));

  auto data = mut.data();

  ASSERT_EQ(contents, std::string(data.begin(), data.end()));
  ASSERT_TRUE(mut == Message(data));
}

TEST(Message, modify_contents)
{
  auto mut =
      Message(std::vector<std::uint8_t>(contents.begin(), contents.end()));

  auto data = mut.data();

  data[0] = 'a';
  ASSERT_TRUE(mut != Message(data));

  mut.data()[0] = 'a';
  ASSERT_FALSE(mut != Message(data));

  ASSERT_EQ(data, mut.data());
}
}
