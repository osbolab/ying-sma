#include "threadscheduler.hh"

#include <cassert>
#include <functional>


namespace sma
{

void ThreadScheduler::do_async(std::function<void()> task)
{
  assert(task);
  threadpool.push([task]() { task(); });
}

void ThreadScheduler::schedule(std::function<void()> task, millis delay_ms)
{
  assert(task);
  LOG(DEBUG) << "task in " << delay_ms.count() << "ms";

  tasks->push(std::move(task), delay_ms);
  tasks_type* const ctasks = tasks.get();
  threadpool.push([ctasks]() { ctasks->pop()(); });
}
}
