#pragma once

#include "Scheduler.hh"

#include <chrono>
#include <cstdlib>
#include <functional>
#include <memory>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>


namespace sma
{

class ThreadScheduler final : public Scheduler
{
  class ThreadTask;

public:
  static Scheduler::Factory* single_threaded_factory();

  ThreadScheduler(std::size_t nr_threads);

  std::shared_ptr<Scheduler::Task>
  schedule_delay(std::function<void* (void*)> f,
                 std::chrono::milliseconds delay,
                 std::size_t times_to_run = 1) override;

private:
  void run_task(std::function<void* (void*)> f, ThreadTask task);

  std::size_t nr_threads;
  // TODO: need a delay queue
  std::queue<std::shared_ptr<ThreadTask>> tasks;
  std::mutex      task_thread_mutex;
  std::thread     task_thread;



  class ThreadTask final : public Scheduler::Task
  {
    friend class ThreadScheduler;
  public:
    ~ThreadTask();

    bool cancel() override;

    void* run_now() override;
    bool  await_result(void*& result_out) override;
    bool  result(void*& result_out) override;

    std::size_t nr_cycles_remaining() override { return nr_cycles_remaining_; }
    bool is_scheduled() override;
    bool is_cancellable() override;
    void decrement_cycles() { --nr_cycles_remaining_; }
    void is_scheduled(bool scheduled) { is_scheduled_ = scheduled; }
    void is_cancellable(bool cancellable) { is_cancellable_ = cancellable; }

  private:
    ThreadTask(ThreadScheduler* scheduler,
               std::function<void* (void*)> f,
               std::chrono::milliseconds delay,
               std::size_t times_to_run = 1);

    void result(void* result_in);

    ThreadScheduler* scheduler;
    std::chrono::milliseconds delay;
    std::function<void* (void*)> f;


    std::mutex state_mutex;
    bool cancel_requested;
    bool is_cancellable_;
    bool is_scheduled_;
    std::size_t nr_cycles_remaining_;

    std::mutex result_mutex;
    std::condition_variable result_available;
    void* result_;
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
};

}