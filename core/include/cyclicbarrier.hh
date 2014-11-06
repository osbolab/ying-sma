#pragma once

#include <cstdlib>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>


namespace sma
{

class CyclicBarrier
{
 public:
  CyclicBarrier(size_t nr_threads);
  CyclicBarrier(size_t nr_threads, std::function<void()>&& onOpened);
  ~CyclicBarrier();

  void wait();

 private:
  std::function<void()>     onOpened;

  const size_t         nr_threads;
  size_t               numWaiting;
  size_t               numToLeave;

  std::mutex                mutex;
  std::condition_variable   closed;
  std::condition_variable   open;

  bool                      forceOpen;
};

}