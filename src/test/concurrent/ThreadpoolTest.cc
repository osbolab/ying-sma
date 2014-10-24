#include "concurrent/Threadpool.hh"

#include "gtest/gtest.h"

#include <iostream>
#include <deque>
#include <random>
#include <chrono>


namespace sma
{

std::atomic<long long> jobms = 0;

void test(int i)
{
  auto ms = std::chrono::milliseconds(20 + std::rand() % 200);
  std::this_thread::sleep_for(ms);
  jobms.fetch_add(ms.count());
}

TEST(Threadpool, AssertionTrue)
{
  Threadpool<> tp(5);
  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 1; i <= 100; ++i) {
    tp.push(test, i);
  }
  tp.join(true);
  auto realms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::high_resolution_clock::now() - start
                );

  ASSERT_LT(realms.count(), jobms.load());
}

TEST(Threadpool_DelayQueue, AssertionTrue)
{

}

}