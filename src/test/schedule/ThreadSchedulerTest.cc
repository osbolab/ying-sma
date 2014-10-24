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

template<typename R, typename ...A>
class Wrapper
{
  using fun_t = std::function<R(A...)>;

public:
  Wrapper(fun_t f)
    : f(f)
  {
  }

private:
  fun_t f;
};

TEST(Schedule_Task, AssertionTrue)
{
#if 0
  auto sched = ThreadScheduler::single_threaded_factory()->new_scheduler();

  auto task = sched->schedule_delay(scheduled_func, std::chrono::milliseconds(1000), 1);

  void* result;
  ASSERT_TRUE(task->await_result(result));

  Packet* p = static_cast<Packet*>(result);
  ASSERT_EQ(15, p->i);
#endif
}

}