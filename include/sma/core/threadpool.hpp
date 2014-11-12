#pragma once

#include <sma/log.hpp>

#include <cstdlib>
#include <vector>
#include <thread>
#include <functional>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>


namespace sma
{

class threadpool
{
public:
  using Task = std::function<void()>;

protected:
  struct shared_state {
    shared_state(std::size_t min_capacity)
      : min_capacity(min_capacity)
      , tasks(min_capacity)
    {
    }

    bool stop{false};
    bool join{false};
    std::mutex mx;
    std::condition_variable available;

    std::size_t min_capacity;
    std::vector<Task> tasks;
    std::size_t p{0};
    std::size_t t{0};
    std::size_t count{0};
    bool scaled{false};
  };

public:
  threadpool(std::size_t nthreads);
  threadpool(std::size_t nthreads, std::size_t initial_capacity);
  threadpool(const threadpool& rhs) = delete;
  threadpool& operator=(const threadpool& rhs) = delete;
  virtual ~threadpool();

  void push_back(Task task);
  void join();

protected:
  virtual void thread_body(shared_state& s);

private:
  static std::size_t resize(shared_state& s, std::size_t len);
  template <typename T>
  static void ringmove(T& dst, T& src, std::size_t src_off, std::size_t len);

  std::vector<std::thread> threads;
  std::unique_ptr<shared_state> s;
};

template <typename T>
void threadpool::ringmove(T& dst, T& src, std::size_t off, std::size_t len)
{
  const std::size_t sz = src.size();
  // Copy the old ring into the front of the new one
  for (std::size_t d = 0; d < len;) {
    dst[d++] = std::move(src[off++]);
    if (off == sz)
      off = 0;
  }
}
}
