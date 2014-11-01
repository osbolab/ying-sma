#include "scheduler.hh"

#include <cassert>


namespace sma
{

void Scheduler::run(detail::taskid_type taskid)
{
  Lock lock(mutex);

  auto const& found = tasks.find(taskid);
  assert(found != tasks.end());

  auto const& task = found->second;
  // The tasks map stores pointers to proxies that call the task with
  // the arguments bound at creation.
  if (task) {
    (*task)();
  }

  tasks.erase(found);
}
}
