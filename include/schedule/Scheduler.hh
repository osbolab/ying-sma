#pragma once

#include <chrono>
#include <cstdlib>
#include <functional>
#include <memory>


namespace sma
{

class Scheduler
{
public:
  class Task;

  virtual ~Scheduler() {}

  virtual std::shared_ptr<Scheduler::Task>
  schedule_delay(std::function<void* (void*)> f,
                 std::chrono::milliseconds delay,
                 std::size_t times_to_run = 1) = 0;



  class Task
  {
  public:
    virtual ~Task() {}

    /*! Attempt to cancel a scheduled task.
    *  If the task is currently executing it will not be
    *  interrupted, but any future executions will be cancelled.
    *  \return \c true if the task was cancelled or \c false if it isn't scheduled.
    */
    virtual bool cancel() = 0;

    /*! Run the task immediately in the calling thread.
     *  \return the result of executing the task.
     */
    virtual void* run_now() = 0;

    /*! Block the calling thread until a result of executing this task is available or the
     *  task is cancelled.
     *  \param result_out contains the result if \c true was returned.
     *  \return \c true if the result was retrieved or \c false if the task was cancelled.
     */
    virtual bool await_result(void*& result_out) = 0;

    /*! Retrieve the result of execution if available.
     *  \param result_out contains the result if \c true was returned.
     *  \return \c true if the result was retrieved or \c false if none is available.
     */
    virtual bool result(void*& result_out) = 0;

    //! \return the number of additional times this task will run.
    virtual std::size_t nr_cycles_remaining() = 0;

    //! \return \c true if this task is scheduled to execute at some point in the future.
    virtual bool is_scheduled() = 0;

    //! \return \c true if the task can be cancelled or \c false if it isn't scheduled.
    virtual bool is_cancellable() = 0;
  };



  class Factory
  {
  public:
    virtual ~Factory() {}

    virtual std::unique_ptr<Scheduler> new_scheduler() const = 0;
  };
};

}