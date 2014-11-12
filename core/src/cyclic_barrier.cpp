#include <sma/concurrent/cyclic_barrier.hpp>

#include <thread>


namespace sma
{

cyclic_barrier::cyclic_barrier(std::size_t nthreads)
  : nthreads(nthreads),
    nwaiting(0),
    nto_leave(0),
    forced_open(false)
{
}

cyclic_barrier::cyclic_barrier(std::size_t nthreads, std::function<void()>&& on_open)
  : nthreads(nthreads),
    on_open(on_open),
    nwaiting(0),
    nto_leave(0),
    forced_open(false)
{
}

cyclic_barrier::~cyclic_barrier()
{
  // Force the barrier open so we don't destroy a locked mutex
  forced_open = true;
  {
    std::unique_lock<std::mutex> lock(mutex);
    if (nwaiting + nto_leave > 0) {
      open.notify_all();
      closed.wait(lock, [&] { return nto_leave == 0; });
    }
  }
}

void cyclic_barrier::wait()
{
  std::unique_lock<std::mutex> lock(mutex);
  // Wait for a recently opened barrier to close again
  closed.wait(lock, [&] { return nto_leave == 0 || forced_open; });
  if (forced_open) return;

  ++nwaiting;
  // The last waiting thread opens the barrier
  if (nwaiting == nthreads) {
    nto_leave = nthreads;
    open.notify_all();
  } else {
    open.wait(lock, [&] { return nwaiting == nthreads || forced_open; });
  }

  --nto_leave;
  // The last leaving thread closes the barrier on its way out
  if (nto_leave == 0) {
    nwaiting = 0;
    closed.notify_all();

    if (on_open) {
      std::thread th(on_open);
      th.detach();
    }
  }
}

}
