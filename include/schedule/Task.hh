#pragma once

#include <cstdlib>
#include <memory>
#include <functional>


namespace sma
{

class Task
{
public:
  struct Wrapper {
    template<typename A>
    A* as()
    {
      static_assert(std::is_base_of<Wrapper, A>::value,
                    "Type pointed to derives from Task::Wrapper");
      return dynamic_cast<A*>(this);
    }

    virtual ~Wrapper() = 0;
  };

  using Ptr = std::unique_ptr<Task::Wrapper>;
  using voidFvoid = std::function<void ()>;
  using ptrFvoid = std::function<Ptr ()>;
  using voidFptr = std::function<void (Ptr)>;
  using ptrFptr = std::function<Ptr (Ptr)>;
  using NullaryCallback = std::function<void(bool, bool)>;
  using UnaryCallback = std::function<void(bool, bool, Ptr)>;

  template<typename A>
  static Ptr ptr(A* a)
  {
    static_assert(std::is_base_of<Wrapper, A>::value,
                  "Ptr type must derive from Task::Wrapper");
    return a ? Ptr(static_cast<Wrapper*>(a)) : nullptr;
  }

  virtual ~Task() {}

  /*! Attempt to cancel a scheduled task.
  *  If the task is currently executing it will not be
  *  interrupted, but any future executions will be cancelled.
  *  \return \c true if the argument was returned.
  */
  virtual bool cancel(Ptr& arg_out) = 0;

  /*! Attempt to modify the argument to the scheduled task.
   *  If it is currently running, then the modification will take effect at the next scheduled
   *  execution or not at all if no further executions are scheduled.
   *  \param  old_args_out contains the old argument pointer if it was changed.
   *  \return \c true if the argument was modified or \c false if it cannot be modified
   *          because the task is not scheduled.
   */
  virtual bool modify_arg(Ptr&& new_args_in,
                          Ptr& old_args_out) = 0;

  /*! Block the calling thread until a result of executing this task is available or the
   *  task is cancelled.
   *  If a callback was specified at the task's scheduling it will \b not be called.
   *  \param result_out contains the result if \c true was returned.
   *  \return \c true if the result was retrieved or \c false if the task was cancelled.
   */
  virtual bool await(Ptr& result_out) = 0;

  /*! Retrieve the result of execution if available.
   *  \param result_out contains the result if \c true was returned.
   *  \return \c true if the result was retrieved or \c false if none is available.
   */
  virtual bool poll(Ptr& result_out) = 0;

  //! \return \c true if the task is not scheduled to run.
  virtual bool is_done() = 0;

  //! \return \c true if the task can be cancelled or \c false if it isn't scheduled.
  virtual bool is_cancellable() = 0;
};

inline Task::Wrapper::~Wrapper() {}

}