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
  CyclicBarrier(std::size_t numThreads);
  CyclicBarrier(std::size_t numThreads, std::function<void()>&& onOpened);
  ~CyclicBarrier();

  void wait();

private:
  std::function<void()>     onOpened;

  const std::size_t         numThreads;
  std::size_t               numWaiting;
  std::size_t               numToLeave;

  std::mutex                mutex;
  std::condition_variable   closed;
  std::condition_variable   open;

  bool                      forceOpen;
};

}