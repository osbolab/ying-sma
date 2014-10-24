#include "schedule/Scheduler.hh"
#include "schedule/ThreadScheduler.hh"

#include "gtest/gtest.h"

#include <chrono>
#include <iostream>
#include <limits>
#include <memory>



namespace sma
{

struct Packet : public Task::Wrapper {
  int i;
};

Task::Ptr scheduled_func(Task::Ptr arg)
{
  if (!arg) return nullptr;

  auto packet_in = arg->as<Packet>();
  auto packet_out = new Packet();
  packet_out->i = 2 * packet_in->i;

  return Task::ptr(packet_out);
}

TEST(Schedule_Task, AssertionTrue)
{
  auto sched = ThreadScheduler::single_threaded_factory()->new_scheduler();

  Packet* arg = new Packet();
  arg->i = 15;

  auto task = sched->schedule(scheduled_func,
                              std::chrono::milliseconds(1000), 1,
                              Task::ptr(arg));

  Task::Ptr result;
  ASSERT_TRUE(task->await(result));
  ASSERT_EQ(30, result->as<Packet>()->i);
}

}