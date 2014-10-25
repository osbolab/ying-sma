#include "schedule/Scheduler.hh"
#include "schedule/ThreadScheduler.hh"
#include "util/Log.hh"

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

void do_announce(std::uint16_t node_id)
{
  auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  LOG_D("Announced at " << now);
}

// function<void(Task, ...A)>
void on_announce_done(std::shared_ptr<Task>& task, std::size_t backoff)
{
  backoff = std::size_t(float(backoff) * 1.2f);
}

TEST(Schedule_Task, AssertionTrue)
{
  auto sched = ThreadScheduler::single_threaded_factory()->new_scheduler();

  auto target = Task::target(do_announce, 32);
  std::future<std::uint16_t> f { target.result->unwrap<std::uint16_t>() };
  std::cout << f.valid() << std::endl;

  //auto task = sched->schedule(do_announce,
}

}