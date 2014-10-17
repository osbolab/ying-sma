#include <iostream>
#include <cstdint>
#include <string>
#include <cassert>
#include <memory>
#include <algorithm>

#include "gtest/gtest.h"

#include "msg/Message.hh"


using namespace sma;
using std::uint8_t;
using std::string;
using std::vector;

TEST(MesageTest, AssertionTrue) {
  string contents("Hello, world!");
  const char* chars = contents.c_str();
  auto mut = MutableMessage(
    std::move(std::vector<std::uint8_t>(contents.begin(), contents.end()))
  );

  auto mutC = MutableMessage(std::vector<std::uint8_t>(chars, chars+contents.size()+1));

  auto data = mut.data();

  std::vector<uint8_t> vec(1);
  vec[0] = '1';
  ASSERT_EQ(vec[0], '1');
  ASSERT_EQ(vec.size(), 1);

  ASSERT_EQ(contents, std::string(data.begin(), data.end()));
  ASSERT_TRUE(mut == MutableMessage(data));

  data[0] = 'a';
  ASSERT_TRUE(mut != MutableMessage(data));

  mut.data()[0] = 'a';
  ASSERT_FALSE(mut != MutableMessage(data));

  // Copy a mutable 
  auto immu = mut;
  auto mutCopy = MutableMessage(immu);
  mutCopy.data().push_back(1);

  auto immuCopy = mutCopy;
  ASSERT_EQ(immuCopy, mutCopy);
  ASSERT_NE(immu, immuCopy);
  ASSERT_NE(immu, mutCopy);

  auto immuMove = Message(std::move(mut));
  ASSERT_EQ(immuMove, mut);

  auto mutMove = Message(std::move(immu));
  ASSERT_EQ(mutMove, immu);
}