#include "concurrent/Threadpool.hh"
#include "concurrent/DelayQueue.hh"
#include "concurrent/thread_interrupted.hh"

#include "gtest/gtest.h"

#include <iostream>
#include <deque>
#include <random>
#include <chrono>
#include <cstdint>
#include <cstdlib>


namespace sma
{

std::atomic<long long> jobms = 0;

using clock = std::chrono::high_resolution_clock;

void test(int i)
{
  auto ms = std::chrono::milliseconds(20 + std::rand() % 50);
  std::this_thread::sleep_for(ms);
  jobms.fetch_add(ms.count());
}

TEST(Threadpool, AssertionTrue)
{
  Threadpool<> tp(5);
  auto start = clock::now();
  for (int i = 1; i <= 100; ++i) {
    tp.push(test, i);
  }
  tp.join(true);
  auto realms = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - start);

  ASSERT_LT(realms.count(), jobms.load());
}

TEST(Delay_Scheduler, AssertionTrue)
{
  std::atomic<bool> stop = false;

  DelayQueue<clock::time_point> dq;

  Threadpool<> tp(5);
  for (std::size_t i = 0; i < tp.nr_threads(); ++i) {
    tp.push([&]() {
      for (;;) {
        if (stop) break;
        try {
          auto waited = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - dq.pop());
          std::cout << waited.count() << " ms" << std::endl;
        } catch (const thread_interrupted& ignore) {
          (void) ignore;
        }
      }
    });
  }

  for (int i = 0; i < 100; ++i) {
    auto wait = std::chrono::milliseconds(10 + std::rand() % 1000);
    dq.push(clock::now(), wait);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(1200));
  stop = true;
  dq.interrupt();
}

}