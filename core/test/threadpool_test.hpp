#pragma once

#include <sma/core/threadpool.hpp>

#include "gtest/gtest.h"

#include <iostream>
#include <sstream>


namespace sma
{

using namespace std::literals::chrono_literals;
using Clock = std::chrono::high_resolution_clock;

template <typename T>
constexpr std::chrono::milliseconds to_ms(T d)
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(d);
}

class threadpool_test : public ::testing::Test
{
protected:
  static std::stringstream output;

  static void SetUpTestCase()
  {
    output << " -------------------------------------------------------\n";
    output << "|   TRIAL   |      TIME       |    COMPARED WITH REAL   |\n";
    output << "|-----------|-----------------|-------------------------|\n";
    output << "| TH  TASKS |  REAL   ACTUAL  |  IDEAL  TARGET  ACTUAL  |\n"
           << std::fixed;
    output << "| --  ----- | ------  ------  |  -----  ------  ------  |\n";
    output.precision(1);
  }
  static void TearDownTestCase()
  {
    output << " ------------------------------------------------------- ";
    std::cout << output.str() << std::endl;
  }

  std::string ti(std::size_t i)
  {
    output << std::setfill(' ') << std::right << i;
    return "";
  }

  std::string p(double p)
  {
    output << std::setw(5) << std::setfill(' ') << std::right << p;
    return "%  ";
  }

  std::string m(int ms)
  {
    output << std::setw(4) << std::setfill(' ') << std::right << ms;
    return "ms";
  }

  template <typename D>
  void do_test(std::size_t nthreads,
               std::size_t ntasks,
               D task_wait,
               double overhead = 0.15)
  {
    threadpool th(nthreads);

    for (int i = 0; i < ntasks; ++i) {
      th.push_back([=]() { std::this_thread::sleep_for(task_wait); });
    }

    auto start = Clock::now();
    th.join();

    auto actual = double(to_ms(Clock::now() - start).count());
    auto real = double(ntasks * task_wait.count());
    auto ideal = real / double(nthreads);
    auto target = (1.0 + overhead) * ideal;

    auto pideal = ideal / real * 100.0;
    auto ptarget = target / real * 100.0;
    auto pactual = actual / real * 100.0;

    output << "| " << std::setw(2) << ti(nthreads) << "   " << std::setw(4)
           << ti(ntasks) << " | " << m(int(real)) << "  " << m(int(actual))
           << "  | " << p(pideal) << p(ptarget) << p(pactual) << "|\n";

    EXPECT_GE(actual, ideal);
    EXPECT_LE(actual, target);
  }
};

std::stringstream threadpool_test::output;


TEST_F(threadpool_test, 1_thread_10_tasks) { do_test(1, 10, 5ms); }
TEST_F(threadpool_test, 1_thread_50_tasks) { do_test(1, 50, 5ms); }
TEST_F(threadpool_test, 1_thread_100_tasks) { do_test(1, 100, 5ms); }
TEST_F(threadpool_test, 2_threads_10_tasks) { do_test(2, 10, 5ms); }
TEST_F(threadpool_test, 2_threads_50_tasks) { do_test(2, 50, 5ms); }
TEST_F(threadpool_test, 2_threads_100_tasks) { do_test(2, 100, 5ms); }
TEST_F(threadpool_test, 4_threads_50_tasks) { do_test(4, 50, 5ms); }
TEST_F(threadpool_test, 4_threads_100_tasks) { do_test(4, 100, 5ms); }
TEST_F(threadpool_test, 4_threads_200_tasks) { do_test(4, 200, 5ms); }
TEST_F(threadpool_test, 8_threads_100_tasks) { do_test(8, 100, 5ms); }
TEST_F(threadpool_test, 8_threads_200_tasks) { do_test(8, 200, 5ms); }
TEST_F(threadpool_test, 8_threads_400_tasks) { do_test(8, 400, 5ms); }
TEST_F(threadpool_test, 8_threads_1000_tasks) { do_test(8, 1000, 5ms); }

#if 0
TEST(threadpool, AssertionTrue)
{
  threadpool<> tp(5);
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

  threadpool<> tp(5);
  for (std::size_t i = 0; i < tp.nthreads(); ++i) {
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

static const std::size_t job_count{100};

using clock = std::chrono::high_resolution_clock;
std::atomic<std::size_t> count{0};
std::mutex mutex;
std::condition_variable finished;

int the_target(int i)
{
  return i + 5;
}

TEST(threadpool, schedule_async)
{
  threadpool<> tp(1);
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
#endif
}
