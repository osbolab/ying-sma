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
  virtual ~Scheduler() {}

  virtual std::shared_ptr<Task>
  schedule(Task::voidFvoid f,
           std::chrono::milliseconds delay = std::chrono::milliseconds(0),
           Task::NullaryCallback on_complete = nullptr) = 0;

  virtual std::shared_ptr<Task>
  schedule(Task::voidFptr f,
           std::chrono::milliseconds delay = std::chrono::milliseconds(0),
           Task::Ptr&& arg = nullptr,
           Task::NullaryCallback on_complete = nullptr) = 0;

  virtual std::shared_ptr<Task>
  schedule(Task::voidFvoid f,
           std::chrono::milliseconds delay = std::chrono::milliseconds(0),
           Task::UnaryCallback on_result = nullptr) = 0;

  virtual std::shared_ptr<Task>
  schedule(Task::ptrFptr f,
           std::chrono::milliseconds delay = std::chrono::milliseconds(0),
           Task::Ptr&& arg = nullptr,
           Task::UnaryCallback on_result = nullptr) = 0;

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