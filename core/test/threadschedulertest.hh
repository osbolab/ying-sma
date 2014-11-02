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
  std::cout << "Factory created" << std::endl;
  auto sched = factory.new_scheduler();
  std::cout << "Scheduler created" << std::endl;

  auto start = clock::now();
  auto task = sched->schedule(delay, do_announce, input);
  std::cout << "Scheduled for " << delay.count() << " millis" << std::endl;
  std::cout << "waiting" << std::endl;
  int result = task->wait();
  std::cout << "dont waiting" << std::endl;
  auto time = clock::now() - start;
  ASSERT_EQ(input, result);
  ASSERT_GT(delay, std::chrono::duration_cast<std::chrono::milliseconds>(time));

  std::cout << "disposing" << std::endl;

  /*
  auto sched = ThreadScheduler::single_threaded_factory()->new_scheduler();

  auto target = Task::target(do_announce, 32);
  std::future<void> f { target.result->unwrap<void>() };
  std::cout << f.valid() << std::endl;
  */
}
}
