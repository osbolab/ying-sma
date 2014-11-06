#pragma once

#include <cstdlib>
#include <chrono>


namespace sma
{

// The application-facing scheduler; has template functions
class Scheduler;


class AbstractScheduler
{
  friend class Scheduler;

public:
  ~AbstractScheduler();

protected:
  virtual void schedule(std::size_t task_id,
                        std::chrono::milliseconds delay) = 0;

  // The scheduler frontend manages the actual tasks that get scheduled
  // We just schedule them by ID.
  Scheduler* tasks;
};
}
