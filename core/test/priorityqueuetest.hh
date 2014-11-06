#include "priorityqueue.hh"

#include "gtest/gtest.h"

#include <iostream>
#include <random>
#include <functional>


namespace sma
{

TEST(PriorityQueue, default_increasing_order)
{
  PriorityQueue<int> q;
  for (int i = 0; i < 1000; ++i) {
    q.push(std::rand());
  }

  while (!q.empty()) {
    int top = q.top();
    int a = q.pop();
    ASSERT_EQ(top, a);
    top = q.top();
    int b = q.pop();
    ASSERT_EQ(top, b);

    ASSERT_LE(a, b);
  }
}



TEST(PriorityQueue, std_greater_decreasing_order)
{
  PriorityQueue<int, std::greater<int>> q;
  for (int i = 0; i < 1000; ++i) {
    q.push(std::rand());
  }

  while (!q.empty()) {
    int top = q.top();
    int a = q.pop();
    ASSERT_EQ(top, a);
    top = q.top();
    int b = q.pop();
    ASSERT_EQ(top, b);

    ASSERT_GE(a, b);
  }
}

TEST(PriorityQueue, clear_empties)
{
  PriorityQueue<int, std::greater<int>> q;
  for (int i = 0; i < 1000; ++i) {
    q.push(std::rand());
  }
  ASSERT_EQ(1000, q.size());
  ASSERT_FALSE(q.empty());
  q.clear();
  ASSERT_EQ(0, q.size());
  ASSERT_TRUE(q.empty());
}
}
