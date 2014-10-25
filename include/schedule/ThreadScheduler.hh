#pragma once

#include "Scheduler.hh"
#include "Task.hh"
#include "concurrent/DelayQueue.hh"

#include <chrono>
#include <cstdlib>
#include <functional>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>


namespace sma
{

class ThreadScheduler final : public Scheduler
{
  class ThreadTask;

  using thread_task_pointer = std::shared_ptr<ThreadTask>;
  using task_q_type = DelayQueue<thread_task_pointer>;
  using f_wrapper = std::function<void()>;

public:
  static Scheduler::Factory* single_threaded_factory();

  ThreadScheduler(std::size_t nr_threads);

  task_pointer
  schedule(Task::Target target, delay_type delay) override;


private:
  template<typename F, typename... Args>
  f_wrapper wrap(F&& f, Args&& ... args);

  std::size_t nr_threads;
  task_q_type  tasks;
  std::mutex  task_thread_mutex;
  std::thread task_thread;



  class ThreadTask final : public Task
  {
    using delay_type = std::chrono::milliseconds;

    friend class ThreadScheduler;

  public:
    ~ThreadTask();

    bool done() override;
    bool cancellable() override;
    void scheduled(bool scheduled) { is_scheduled = scheduled; }
    void cancellable(bool cancellable) { is_cancellable = cancellable; }

  private:
    ThreadTask(ThreadScheduler* scheduler,
               Task::Target target,
               delay_type delay);

    void run();

    ThreadScheduler* scheduler;
    delay_type delay;
    Task::Target target;

    bool cancel_requested;
    bool is_cancellable;
    bool is_scheduled;

    std::mutex mutex;
    std::condition_variable result_available;
  };



  class Factory final : public Scheduler::Factory
  {
  public:
    Factory(std::size_t nr_threads);
    std::unique_ptr<Scheduler> new_scheduler() const override;

  private:
    const std::size_t nr_threads;
  };

  static Factory* const single_threaded_factory_;


private:
};

}