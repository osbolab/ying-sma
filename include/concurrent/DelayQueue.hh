#pragma once

#include "concurrent/thread_interrupted.hh"

#include <queue>
#include <chrono>
#include <mutex>
#include <condition_variable>


namespace sma
{

namespace detail
{
template<typename E, typename Clock>
class Wrapper;
}

template<typename E,
         typename Clock = std::chrono::high_resolution_clock>
class DelayQueue final
{
  using Myt = DelayQueue<E, Clock>;
  using wrapper_type = detail::Wrapper<E, Clock>;
  using Lock = std::unique_lock<std::mutex>;
  using queue_type =
    std::priority_queue<wrapper_type, std::deque<wrapper_type>, std::greater<wrapper_type>>;

public:
  DelayQueue()
    : interrupted(false), nr_waiting(0)
  {
  }

  DelayQueue(const Myt& copy)
    : q(copy.q)
  {
  }

  DelayQueue(Myt&& move)
    : q(std::move(move.q))
  {
  }

  template<typename Delay>
  void push(const E& e, Delay delay)
  {
    push(wrapper_type(e, delay));
  }

  template<typename Delay>
  void push(E&& e, Delay delay)
  {
    push(wrapper_type(std::move(e), delay));
  }

  E pop()
  {
    Lock lock(mutex);

    for (;;) {
      if (interrupted) {
        interrupted = (nr_waiting != 0);
        throw thread_interrupted();
      }
      if (q.empty()) {
        ++nr_waiting;
        available.wait(lock);
        --nr_waiting;
      } else {
        const wrapper_type* first = &(q.top());
        if (!first->expired()) {
          ++nr_waiting;
          available.wait_for(lock, first->delay());
          --nr_waiting;
        } else {
          auto actual = std::move(q.top());
          q.pop();
          if (!q.empty()) available.notify_all();
          return std::move(actual.entry);
        }
      }
    }
  }

  std::size_t size()
  {
    Lock lock(mutex);
    return q.size();
  }

  bool empty()
  {
    Lock lock(mutex);
    return q.empty();
  }

  void interrupt()
  {
    Lock lock(mutex);
    if (nr_waiting > 0) {
      interrupted = true;
      available.notify_all();
    }
  }

  Myt& operator =(const Myt& copy)
  {
    q = copy.q;
    return *this;
  }

  Myt& operator =(Myt&& move)
  {
    q = std::move(move.q);
    return *this;
  }

private:
  void push(wrapper_type&& delayed)
  {
    Lock lock(mutex);
    const wrapper_type* first = !q.empty() ? &(q.top()) : nullptr;
    q.push(std::move(delayed));
    // notify pop() that there's a sooner delay than the one it's waiting on
    if (!first || delayed < *first) available.notify_all();
  }


  queue_type q;
  std::mutex mutex;
  std::condition_variable available;
  bool interrupted { false };
  std::size_t nr_waiting { 0 };
};


namespace detail
{

template<typename E, typename Clock>
class Wrapper final
{
  template<typename E, typename Clock>
  friend class DelayQueue;

  using Myt = Wrapper<E, Clock>;
  using delay_type = typename Clock::duration;
  using time_point = typename Clock::time_point;

public:
  Wrapper(Myt&& move)
    : entry(std::move(move.entry)), delay_until(move.delay_until)
  {
  }

  Myt& operator =(Myt&& move)
  {
    std::swap(entry, move.entry);
    delay_until = move.delay_until;
    return *this;
  }

  bool operator >(const Myt& rhs) const { return delay() > rhs.delay(); }
  bool operator <(const Myt& rhs) const { return delay() < rhs.delay(); }
  bool operator >=(const Myt& rhs) const { return delay() >= rhs.delay(); }
  bool operator <=(const Myt& rhs) const { return delay() <= rhs.delay(); }

private:
  static time_point project_future(delay_type delay)
  {
    return Clock::now() + delay;
  }

  template<typename Delay>
  Wrapper(const E& entry, Delay delay)
    :entry(entry), delay_until(project_future(delay))
  {
  }

  template<typename Delay>
  Wrapper(E&& entry, Delay delay)
    : entry(std::move(entry)), delay_until(project_future(delay))
  {
  }

  delay_type delay() const
  {
    return delay_until - Clock::now();
  }

  bool expired() const
  {
    return delay().count() <= 0;
  }

  Wrapper(const Myt& copy) = delete;
  Myt& operator =(const Myt& copy) = delete;


  E entry;
  time_point delay_until;
};

} // namespace detail

} // namespace sma