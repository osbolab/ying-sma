#pragma once

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
  using lock_type = std::unique_lock<std::mutex>;
  using queue_type =
    std::priority_queue<wrapper_type, std::deque<wrapper_type>, std::greater<wrapper_type>>;

public:
  DelayQueue()
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

  template<typename DelayT>
  void push(const E& e, DelayT delay)
  {
    push(wrapper_type(e, delay));
  }

  template<typename DelayT>
  void push(E&& e, DelayT delay)
  {
    push(wrapper_type(std::move(e), delay));
  }

  E take()
  {
    lock_type lock(mutex);

    for (;;) {
      if (q.empty()) available.wait(lock);
      else {
        const wrapper_type* first = &(q.top());
        if (!first->expired()) {
          available.wait_for(lock, first->delay());
        } else {
          auto actual = std::move(q.top());
          q.pop();
          if (!q.empty()) available.notify_all();
          return std::move(actual.entry);
        }
      }
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
    lock_type lock(mutex);
    const wrapper_type* first = !q.empty() ? &(q.top()) : nullptr;
    q.push(std::move(delayed));
    // notify take() that there's a sooner delay than the one it's waiting on
    if (!first || delayed < *first) available.notify_all();
  }


  queue_type q;
  std::mutex mutex;
  std::condition_variable available;
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

  template<typename DelayT>
  Wrapper(const E& entry, DelayT delay)
    :entry(entry), delay_until(project_future(delay))
  {
  }

  template<typename DelayT>
  Wrapper(E&& entry, DelayT delay)
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