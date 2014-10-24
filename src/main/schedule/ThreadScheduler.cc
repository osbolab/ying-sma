#include "schedule/ThreadScheduler.hh"
#include "schedule/Task.hh"

#include "util/Log.hh"

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
  return std::unique_ptr<Scheduler>(new ThreadScheduler(nr_threads));
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

std::shared_ptr<Task>
ThreadScheduler::schedule(Task::voidFptr f,
                          std::chrono::milliseconds delay,
                          Task::Ptr&& arg,
                          Task::Callback on_result)
{
  auto task = std::shared_ptr<ThreadTask>(
                new ThreadTask(this, f, delay, std::move(arg), on_result)
              );
  task->is_scheduled(true);
  task->is_cancellable(true);

  tasks.push(task);

  {
    std::unique_lock<std::mutex> lock(task_thread_mutex);
    if (!task_thread.joinable()) {
      task_thread = std::thread([this]() {
        LOG_D("[ThreadScheduler::{runner}] task thread is alive");
        while (!tasks.empty()) {
          auto task = std::move(tasks.front());
          tasks.pop();
          LOG_D("[ThreadScheduler::{runner}] sleeping for " << task->delay.count() << "ms");
          std::this_thread::sleep_for(task->delay);
          LOG_D("[ThreadScheduler::{runner}] awake");
          {
            std::unique_lock<std::mutex> lock(task->state_mutex);
            if (task->cancel_requested) {
              LOG_D("[ThreadScheduler::{runner}] task cancelled");
            }
            task->is_cancellable(false);
          }
          LOG_D("[ThreadScheduler::{runner}] running task");
          task->set_result(std::move(task->f(nullptr)));
          {
            std::unique_lock<std::mutex> lock(task->state_mutex);
            task->decrement_cycles();
            if (task->nr_cycles_remaining() > 0) {
              task->is_cancellable(true);
              tasks.push(std::move(task));
            } else {
              task->is_scheduled(false);
            }
          }
        }
        LOG_D("[ThreadScheduler::{runner}] task thread dying");
        {
          std::unique_lock<std::mutex> lock(task_thread_mutex);
          if (this->task_thread.joinable()) {
            LOG_D("[ThreadScheduler::{runner}] no task thread");
            this->task_thread = std::thread();
          }
        }
      });
    }
  }

  return task;
}



/*******************************************************************************
 * ThreadTask
 */

ThreadScheduler::ThreadTask::ThreadTask(ThreadScheduler* scheduler,
                                        std::chrono::milliseconds delay,
                                        std::unique_ptr<Caller>&& caller)
  : scheduler(scheduler),
    delay(delay),
    caller(std::move(caller)),
    cancel_requested(false), is_cancellable_(false), is_scheduled_(false)
{
  LOG_D("[ThreadTask::()] " << delay.count() << "ms");
}

ThreadScheduler::ThreadTask::~ThreadTask()
{
  LOG_D("[ThreadTask::~]");
}

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

bool
ThreadScheduler::ThreadTask::is_done()
{
  std::unique_lock<std::mutex> lock(state_mutex);
  return !is_scheduled_;
}

bool
ThreadScheduler::ThreadTask::is_cancellable()
{
  std::unique_lock<std::mutex> lock(state_mutex);
  return is_cancellable_;
}

/* ThreadTask
 ******************************************************************************/


}
