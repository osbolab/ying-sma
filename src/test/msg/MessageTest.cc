#include <iostream>
#include <cstdint>
#include <string>
#include <cassert>

#include "gtest/gtest.h"

#include "msg/Message.hh"


using sma::Message;
using std::uint8_t;
using std::string;
using std::vector;

TEST(MesageTest, AssertionTrue) {
  string contents("Hello, world!");
  Message msg = Message(
    reinterpret_cast<const uint8_t*>(contents.c_str()),
    contents.size() + 1
  );

  const vector<const uint8_t> data = msg.getData();

  ASSERT_EQ(data.size(), contents.length()+1);

  string sdata(data.begin(), data.end()-1);

  ASSERT_EQ(sdata, contents);
}