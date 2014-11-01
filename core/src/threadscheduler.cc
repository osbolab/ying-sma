#include "threadscheduler.hh"

#include "log.hh"

#include <chrono>
#include <thread>
#include <cstdlib>
#include <iostream>
#include <functional>


namespace sma
{

void ThreadScheduler::schedule(detail::taskid_type taskid, millis delay)
{
  Lock lock(mutex);
  tasks.push(taskid, delay);
}
}
