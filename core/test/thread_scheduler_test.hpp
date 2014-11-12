#include <sma/core/thread_scheduler.hpp>

#include <sma/log.hpp>

#include "gtest/gtest.h"

#include <chrono>
#include <iostream>
#include <limits>
#include <cstdlib>
#include <cstdint>
#include <memory>
#include <ctime>
#include <iomanip>



namespace sma
{


int do_announce(std::uint32_t node_id)
{
  LOG(DEBUG) << "Success! Task callback called";
  return node_id;
}

/*
// function<void(Task, ...A)>
void on_announce_done(std::shared_ptr<Task>& task, std::size_t backoff)
{
  backoff = std::size_t(float(backoff) * 1.2f);
}
*/

TEST(scheduler, schedule_task)
{
  using clock = std::chrono::high_resolution_clock;

  auto const delay = 100_ms;
  const std::uint32_t input = 15;

  auto factory = thread_scheduler::Factory<1>();
  auto sched = factory.new_scheduler();
  auto start = clock::now();
  auto task = sched->schedule(delay, do_announce, input);
  LOG(DEBUG) << "scheduled task; waiting for result";
  int result = task->wait();
  auto time = clock::now() - start;
  LOG(DEBUG) << "task finished: " << result;
  ASSERT_EQ(input, result);
  ASSERT_LE(delay.count(), std::chrono::duration_cast<std::chrono::milliseconds>(time).count());
}
}
