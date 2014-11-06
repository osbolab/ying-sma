#include "threadpool.hh"
#include "delayqueue.hh"
#include "thread_interrupted.hh"
#include "log.hh"

#include "gtest/gtest.h"

#include <iostream>
#include <deque>
#include <random>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <thread>
#include <condition_variable>
#include <mutex>


namespace sma
{

#if 0
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
#endif

std::atomic<unsigned long long> jobms{0};

static const std::size_t job_count{100};

using clock = std::chrono::high_resolution_clock;
std::atomic<std::size_t> count{0};
std::mutex mutex;
std::condition_variable finished;

int the_target(int i)
{
  return i + 5;
}

TEST(Threadpool, schedule_async)
{
  Threadpool<> tp(1);
  tp.push(the_target, 5);

  std::atomic<bool> stop{false};

  DelayQueue<clock::time_point> dq;

  std::vector<std::future<void>> futures;

  for (std::size_t i = 0; i < 5; ++i) {
    futures.push_back(std::async(std::launch::async, [&]() {
      for (;;) {
        if (stop)
          break;
        try {
          auto waited = std::chrono::duration_cast<std::chrono::milliseconds>(
              clock::now() - dq.pop());
          jobms.fetch_add(waited.count());
          if (count.fetch_add(1) >= job_count - 1) {
            std::unique_lock<std::mutex> lock(mutex);
            finished.notify_all();
          }
        } catch (const thread_interrupted& ignore) {
          (void) ignore;
        }
      }
    }));
  }

  auto start = clock::now();

  for (int i = 0; i < job_count; ++i) {
    auto wait = std::chrono::milliseconds{10 + std::rand() % 1000};
    dq.push(clock::now(), wait);
  }

  while (count.load() < job_count) {
    std::unique_lock<std::mutex> lock(mutex);
    finished.wait(lock, []() { return count.load() >= 100; });
  }
  stop = true;
  dq.interrupt();

  for (auto& fut : futures) {
    if (fut.valid())
      fut.wait();
  }

  auto realms = std::chrono::duration_cast<std::chrono::milliseconds>(
      clock::now() - start);

  LOG(DEBUG) << count.load() << " jobs totaling "
                     << std::chrono::milliseconds(jobms.load()).count()
                     << " ms in " << std::chrono::milliseconds(realms).count()
                     << " real ms";
}
}
