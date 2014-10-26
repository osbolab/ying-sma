#include "Scheduler2.hh"
//#include "schedule/ThreadScheduler.hh"
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
#include <literals>



namespace sma
{


int do_announce(std::uint16_t node_id)
{
  auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  LOG_D("Announced at " << now);
}

/*
// function<void(Task, ...A)>
void on_announce_done(std::shared_ptr<Task>& task, std::size_t backoff)
{
  backoff = std::size_t(float(backoff) * 1.2f);
}
*/

TEST(Schedule_Task, AssertionTrue)
{
  auto sched = Scheduler();
  sched.schedule(literal, do_announce, 15);

  Task<int> t(do_announce, 15);
  /*
  auto sched = ThreadScheduler::single_threaded_factory()->new_scheduler();

  auto target = Task::target(do_announce, 32);
  std::future<void> f { target.result->unwrap<void>() };
  std::cout << f.valid() << std::endl;
  */
}

}