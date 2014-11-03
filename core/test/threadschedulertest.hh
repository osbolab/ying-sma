#include "threadscheduler.hh"
#include "chrono_literals.hh"

#include "log.hh"
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
  auto now =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  LOG(DEBUG) << "Announced at " << now;
  return 0;
}

/*
// function<void(Task, ...A)>
void on_announce_done(std::shared_ptr<Task>& task, std::size_t backoff)
{
  backoff = std::size_t(float(backoff) * 1.2f);
}
*/

TEST(Scheduler, schedule_task)
{
  using clock = std::chrono::high_resolution_clock;

  auto const delay = 100_ms;
  const std::uint32_t input = 15;

  auto factory = ThreadScheduler::Factory<1>();
  auto sched = factory.new_scheduler();
  auto start = clock::now();
  auto task = sched->schedule(delay, do_announce, input);
  LOG(DEBUG) << "scheduled task; waiting for result";
  //int result = task->wait();
  auto time = clock::now() - start;
  //LOG(DEBUG) << "task finished: " << result;
  //ASSERT_EQ(input, result);
  //ASSERT_GT(delay, std::chrono::duration_cast<std::chrono::milliseconds>(time));

  LOG(DEBUG) << "waiting for terminal input";

  std::cout << "Press Enter to continue";
  std::cin.ignore();
}
}
