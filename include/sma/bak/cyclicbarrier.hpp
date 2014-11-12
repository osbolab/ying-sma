#pragma once

#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <functional>


namespace sma
{

class CyclicBarrier
{
 public:
  CyclicBarrier(std::size_t nr_threads);
  CyclicBarrier(std::size_t nr_threads, std::function<void()>&& onOpened);
  ~CyclicBarrier();

  void wait();

 private:
  std::function<void()>     onOpened;

  const std::size_t         nr_threads;
  std::size_t               numWaiting;
  std::size_t               numToLeave;

  std::mutex                mutex;
  std::condition_variable   closed;
  std::condition_variable   open;

  bool                      forceOpen;
};

}
