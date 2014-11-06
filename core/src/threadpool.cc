#include "threadpool.hh"



namespace sma
{

void Threadpool::add_thread()
{
  Lock lock_threads(state->threads_mutex);

  // Take a thread-local copy of this->state that won't be lost
  // if this object gets moved.
  shared_state* const cstate = state.get();
  cstate->threads.emplace_back([cstate] {
    LOG(DEBUG) << "child thread spawned";
    for (;;) {
      std::function<void()> task;

      {
        Lock lock_tasks{cstate->tasks_mutex};
        if (cstate->tasks.empty())
          cstate->available.wait(lock_tasks, [cstate] {
            return cstate->stop || !cstate->tasks.empty();
          });
        if (cstate->stop && cstate->tasks.empty())
          break;
        task = std::move(cstate->tasks.front());
        cstate->tasks.pop_back();
      }
      if (!cstate->tasks.empty())
        cstate->available.notify_one();

      task();

      {
        Lock lock_threads{cstate->threads_mutex};
        if (cstate->tasks.size() < cstate->threads.size()
            && cstate->threads.size() > cstate->nr_core_threads)
          break;
      }
    }
    LOG(DEBUG) << "child thread dead";
  });
}


void Threadpool::shutdown()
{
  shared_state* const cstate = state.get();
  if (cstate) {
    LOG(DEBUG);
    {
      Lock lock{cstate->tasks_mutex};
      cstate->stop = true;
    }
    cstate->available.notify_all();
  }
}


void Threadpool::join()
{
  shared_state* const cstate = state.get();
  if (cstate) {
    LOG(DEBUG) << "joining " << cstate->threads.size() << " threads...";
    {
      Lock lock{cstate->threads_mutex};
      for (std::thread& thread : cstate->threads)
        if (thread.joinable())
          thread.join();
    }
    LOG(DEBUG) << "... joined";
  }
}
}
