#include "cyclicbarrier.hh"

#include <thread>


namespace sma
{

CyclicBarrier::CyclicBarrier(std::size_t nr_threads)
  : nr_threads(nr_threads),
    numWaiting(0),
    numToLeave(0),
    forceOpen(false)
{
}

CyclicBarrier::CyclicBarrier(std::size_t nr_threads, std::function<void()>&& onOpened)
  : nr_threads(nr_threads),
    onOpened(onOpened),
    numWaiting(0),
    numToLeave(0),
    forceOpen(false)
{
}

CyclicBarrier::~CyclicBarrier()
{
  // Force the barrier open so we don't destroy a locked mutex
  forceOpen = true;
  {
    std::unique_lock<std::mutex> lock(mutex);
    if (numWaiting + numToLeave > 0) {
      open.notify_all();
      closed.wait(lock, [&] { return numToLeave == 0; });
    }
  }
}

void CyclicBarrier::wait()
{
  std::unique_lock<std::mutex> lock(mutex);
  // Wait for a recently opened barrier to close again
  closed.wait(lock, [&] { return numToLeave == 0 || forceOpen; });
  if (forceOpen) return;

  ++numWaiting;
  // The last waiting thread opens the barrier
  if (numWaiting == nr_threads) {
    numToLeave = nr_threads;
    open.notify_all();
  } else {
    open.wait(lock, [&] { return numWaiting == nr_threads || forceOpen; });
  }

  --numToLeave;
  // The last leaving thread closes the barrier on its way out
  if (numToLeave == 0) {
    numWaiting = 0;
    closed.notify_all();

    if (onOpened) {
      std::thread th(onOpened);
      th.detach();
    }
  }
}

}
