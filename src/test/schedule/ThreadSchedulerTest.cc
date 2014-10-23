#include "schedule/Scheduler.hh"
#include "schedule/ThreadScheduler.hh"

#include "gtest/gtest.h"

#include <chrono>
#include <iostream>
#include <limits>


namespace sma
{

struct Packet {
  int i;
};

void* scheduled_func(void* arg)
{
  Packet* p = new Packet();
  p->i = 15;

  return static_cast<void*>(p);
}

TEST(Schedule_Task, AssertionTrue)
{
  auto sched = ThreadScheduler::single_threaded_factory()->new_scheduler();

  auto task = sched->schedule_delay(scheduled_func, std::chrono::milliseconds(1000), 1);

  void* result;
  ASSERT_TRUE(task->await_result(result));

  Packet* p = static_cast<Packet*>(result);
  ASSERT_EQ(15, p->i);
}

}