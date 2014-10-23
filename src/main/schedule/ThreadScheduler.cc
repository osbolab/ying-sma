#include "schedule/ThreadScheduler.hh"

#include "Log.hh"

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

ThreadScheduler::Factory::Factory(std::size_t nr_threads) : nr_threads(nr_threads) {}

std::unique_ptr<Scheduler>
ThreadScheduler::Factory::new_scheduler() const
{
  return std::unique_ptr<Scheduler>(new ThreadScheduler(nr_threads));
}

ThreadScheduler::Factory* const
ThreadScheduler::single_threaded_factory_ = new ThreadScheduler::Factory(1);

Scheduler::Factory*
ThreadScheduler::single_threaded_factory() {return single_threaded_factory_; }

/* Factory
 ******************************************************************************/

ThreadScheduler::ThreadScheduler(std::size_t nr_threads)
  : nr_threads(nr_threads)
{
}

std::shared_ptr<Scheduler::Task>
ThreadScheduler::schedule_delay(std::function<void* (void*)> f,
                                std::chrono::milliseconds delay,
                                std::size_t times_to_run)
{
  auto task = std::shared_ptr<ThreadTask>(new ThreadTask(this, f, delay, times_to_run));
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
          task->result(task->f(nullptr));
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
                                        std::function<void* (void*)> f,
                                        std::chrono::milliseconds delay,
                                        std::size_t times_to_run)
  : scheduler(scheduler), delay(delay), f(f), result_(nullptr),
    cancel_requested(false), is_cancellable_(false), is_scheduled_(false),
    nr_cycles_remaining_(times_to_run)
{
  LOG_D("[ThreadTask::()] " << delay.count() << "ms");
}

ThreadScheduler::ThreadTask::~ThreadTask()
{
  LOG_D("[ThreadTask::~]");
}

bool
ThreadScheduler::ThreadTask::cancel()
{
  LOG_D("[ThreadTask::cancel]");
  std::unique_lock<std::mutex> lock(state_mutex);
  if (!is_cancellable_) return false;
  cancel_requested = true;
  result_ = nullptr;
  return true;
}

void*
ThreadScheduler::ThreadTask::run_now()
{
  LOG_D("[ThreadTask::run_now]");
  return f(nullptr);
}

void
ThreadScheduler::ThreadTask::result(void* result_in)
{
  LOG_D("[ThreadTask::set_result]");
  {
    std::unique_lock<std::mutex> lock(state_mutex);
    result_ = result_in;
  }
  if (result_ != nullptr) {
    result_available.notify_all();
  }
}

bool
ThreadScheduler::ThreadTask::result(void*& result_out)
{
  LOG_D("[ThreadTask::get_result]");
  std::unique_lock<std::mutex> lock(state_mutex);
  if (result_ != nullptr) {
    result_out = result_;
    return true;
  }
  return false;
}

bool
ThreadScheduler::ThreadTask::await_result(void*& result_out)
{
  LOG_D("[ThreadTask::await_result]");
  std::unique_lock<std::mutex> lock(state_mutex);
  while (!result_) {
    result_available.wait(lock, [&]() {
      return result_ || cancel_requested || !is_scheduled_;
    });
    if (cancel_requested || (!is_scheduled_ && !result_)) return false;
  }
  result_out = result_;
  return true;
}

bool
ThreadScheduler::ThreadTask::is_scheduled()
{
  std::unique_lock<std::mutex> lock(state_mutex);
  return is_scheduled_;
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
