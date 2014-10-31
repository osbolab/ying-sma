#pragma once

#include "Task.hh"

#include <chrono>
#include <cstdlib>
#include <functional>
#include <memory>


namespace sma
{

class Scheduler
{
public:
  using task_pointer = std::shared_ptr<Task>;
  using delay_type = std::chrono::milliseconds;


  virtual ~Scheduler() {}

  virtual task_pointer
  schedule(Task::Target target, delay_type delay) = 0;

protected:


public:
  class Factory
  {
  public:
    virtual ~Factory() {}

    virtual std::unique_ptr<Scheduler> new_scheduler() const = 0;
  };
};

}