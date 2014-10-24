#include "DelayQueue.hh"

#include "gtest/gtest.h"

#include <iostream>
#include <memory>
#include <chrono>
#include <deque>


namespace sma
{

class Dummy
{
public:
  Dummy(int i) : i(i) {}

  int i;
};

TEST(Move_Semantics, AssertionTrue)
{
  DelayQueue<std::unique_ptr<Dummy>> dq;

  dq.push(std::unique_ptr<Dummy>(new Dummy(2)), std::chrono::microseconds(2000));
  dq.push(std::unique_ptr<Dummy>(new Dummy(100)), std::chrono::milliseconds(100));
  dq.push(std::unique_ptr<Dummy>(new Dummy(20)), std::chrono::milliseconds(20));

  auto d = dq.take();
  ASSERT_EQ(2,  d->i);

  d = dq.take();
  ASSERT_EQ(20,  d->i);

  d = dq.take();
  ASSERT_EQ(100,  d->i);
}

TEST(Copy_Semantics, AssertionTrue)
{
  DelayQueue<int> dq;

  int x=2, y=20, z=100;

  dq.push(x, std::chrono::microseconds(2000));
  dq.push(y, std::chrono::milliseconds(20));
  dq.push(z, std::chrono::milliseconds(100));

  auto d = dq.take();
  ASSERT_EQ(x,  d);

  d = dq.take();
  ASSERT_EQ(y,  d);

  d = dq.take();
  ASSERT_EQ(z,  d);
}

}