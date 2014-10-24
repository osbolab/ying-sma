#pragma once

#include "Scheduler.hh"
#include "Task.hh"

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

  std::shared_ptr<Task>
  schedule(Task::voidFvoid f,
           std::chrono::milliseconds delay = std::chrono::milliseconds(0),
           Task::NullaryCallback on_complete = nullptr) override;

  std::shared_ptr<Task>
  schedule(Task::voidFptr f,
           std::chrono::milliseconds delay = std::chrono::milliseconds(0),
           Task::Ptr&& arg = nullptr,
           Task::NullaryCallback on_complete = nullptr) override;

  std::shared_ptr<Task>
  schedule(Task::voidFvoid f,
           std::chrono::milliseconds delay = std::chrono::milliseconds(0),
           Task::UnaryCallback on_result = nullptr) override;

  std::shared_ptr<Task>
  schedule(Task::ptrFptr f,
           std::chrono::milliseconds delay = std::chrono::milliseconds(0),
           Task::Ptr&& arg = nullptr,
           Task::UnaryCallback on_result = nullptr) override;


private:
  struct Caller;

  void run_task(Task::voidFvoid f, ThreadTask task);

  std::size_t nr_threads;
  // TODO: need a delay queue
  std::queue<std::shared_ptr<ThreadTask>> tasks;
  std::mutex      task_thread_mutex;
  std::thread     task_thread;



  class ThreadTask final : public Task
  {
    friend class ThreadScheduler;

  public:
    ~ThreadTask();

    bool cancel(Task::Ptr& arg_out) override;

    bool  modify_arg(Task::Ptr&& new_arg_in, Task::Ptr& old_arg_out) override;

    bool  await(Task::Ptr& result_out) override;
    bool  poll(Task::Ptr& result_out) override;

    bool is_done() override;
    bool is_cancellable() override;
    void is_scheduled(bool scheduled) { is_scheduled_ = scheduled; }
    void is_cancellable(bool cancellable) { is_cancellable_ = cancellable; }

  private:
    ThreadTask(ThreadScheduler* scheduler,
               std::chrono::milliseconds delay,
               std::unique_ptr<Caller>&& caller);

    void run();

    ThreadScheduler* scheduler;
    std::chrono::milliseconds delay;
    Task::voidFvoid f;

    std::mutex state_mutex;
    bool cancel_requested;
    bool is_cancellable_;
    bool is_scheduled_;

    std::mutex result_mutex;
    std::condition_variable result_available;

    std::unique_ptr<Caller> caller;
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
  struct Caller {
    Caller() : result(nullptr) {}
    virtual ~Caller() {}

    virtual void call() = 0;

    Task::Ptr result;
  };

  struct voidFvoidCaller final : public Caller {
    void call() override
    {
      if (f) f();
    }
  private:
    Task::voidFvoid f;
  };

  struct voidFptrCaller final : public Caller {
    void call() override
    {
      if (f) f(std::move(arg));
    }
  private:
    Task::voidFptr f;
    Task::Ptr arg;
  };

  struct ptrFvoidCaller final : public Caller {
    void call() override
    {
      if (f) result = f();
    }
  private:
    Task::ptrFvoid f;
  };

  struct ptrFptrCaller final : public Caller {
    void call() override
    {
      if (f) result = f(std::move(arg));
    }
  private:
    Task::ptrFptr f;
    Task::Ptr arg;
  };
};

}