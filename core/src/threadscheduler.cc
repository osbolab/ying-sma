#include "threadscheduler.hh"
#include "task.hh"

#include "log.hh"

#include <chrono>
#include <thread>
#include <cstdlib>
#include <iostream>
#include <functional>


namespace sma
{

/*******************************************************************************
 * Factory
 */

ThreadScheduler::Factory::Factory(std::size_t nr_threads)
  : nr_threads(nr_threads)
{
}

std::unique_ptr<Scheduler>
ThreadScheduler::Factory::new_scheduler() const
{
  return std::unique_ptr<Scheduler> {new ThreadScheduler(nr_threads)};
}

ThreadScheduler::Factory* const
ThreadScheduler::single_threaded_factory_ = new ThreadScheduler::Factory(1);

Scheduler::Factory*
ThreadScheduler::single_threaded_factory()
{
  return single_threaded_factory_;
}

/* Factory
 ******************************************************************************/

ThreadScheduler::ThreadScheduler(std::size_t nr_threads)
  : nr_threads(nr_threads)
{
}

Scheduler::task_pointer
ThreadScheduler::schedule(Task::Target target, delay_type delay)
{
  auto task = std::shared_ptr<ThreadTask> {
    std::move(new ThreadTask(this, std::move(target), delay))
  };
  task->scheduled(true);
  task->cancellable(true);

  tasks.push(task, delay);

  return task;
}


/*******************************************************************************
 * ThreadTask
 */

ThreadScheduler::ThreadTask::ThreadTask(ThreadScheduler* scheduler,
                                        Task::Target target,
                                        std::chrono::milliseconds delay)
  : scheduler(scheduler),
    target(std::move(target)),
    delay(delay),
    cancel_requested(false), is_cancellable(false), is_scheduled(false)
{
  LOG_D("[ThreadTask::()] " << delay.count() << "ms");
}

ThreadScheduler::ThreadTask::~ThreadTask()
{
  LOG_D("[ThreadTask::~]");
}


bool
ThreadScheduler::ThreadTask::done()
{
  std::unique_lock<std::mutex> lock { mutex };
  return !is_scheduled;
}

bool
ThreadScheduler::ThreadTask::cancellable()
{
  std::unique_lock<std::mutex> lock { mutex };
  return is_cancellable;
}

#if 0
void
ThreadScheduler::ThreadTask::set_result(Task::Ptr&& result_in)
{
  LOG_D("[ThreadTask::set_result]");
  {
    std::unique_lock<std::mutex> lock(state_mutex);
    result = std::move(result_in);
  }
  result_available.notify_all();
}

bool
ThreadScheduler::ThreadTask::cancel(Task::Ptr& arg_out)
{
  LOG_D("[ThreadTask::cancel]");
  std::unique_lock<std::mutex> lock(state_mutex);
  if (!is_cancellable_) return false;
  cancel_requested = true;
  if (arg) {
    arg_out = std::move(arg);
    return true;
  }
  arg_out = nullptr;
  return false;
}

bool
ThreadScheduler::ThreadTask::modify_arg(Task::Ptr&& new_arg_in, Task::Ptr& old_arg_out)
{
  if (is_done()) return false;
  old_arg_out = std::move(arg);
  arg = std::move(new_arg_in);
  return true;
}

bool
ThreadScheduler::ThreadTask::get(Task::Ptr& result_out)
{
  LOG_D("[ThreadTask::get_result]");
  std::unique_lock<std::mutex> lock(state_mutex);
  if (wrapper-> != nullptr) {
    result_out = std::move(result);
    return true;
  }
  return false;
}

bool
ThreadScheduler::ThreadTask::await(Task::Ptr& result_out)
{
  LOG_D("[ThreadTask::await_result]");
  std::unique_lock<std::mutex> lock(state_mutex);
  while (!result) {
    result_available.wait(lock, [&]() {
      return result || cancel_requested || !is_scheduled_;
    });
    if (cancel_requested || (!is_scheduled_ && !result)) return false;
  }
  result_out = std::move(result);
  return true;
}
#endif

/* ThreadTask
 ******************************************************************************/


}
