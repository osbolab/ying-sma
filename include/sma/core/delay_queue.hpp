#pragma once

#include <sma/core/priority_queue.hpp>
#include <sma/core/thread_interrupted.hpp>
#include <sma/log.hpp>

#include <utility>
#include <chrono>
#include <mutex>
#include <condition_variable>


namespace sma
{

template <typename E, typename Clock>
class Wrapper final
{
  template <typename E_, typename Clock_>
  friend class DelayQueue;

  using Myt = Wrapper<E, Clock>;
  using delay_type = typename Clock::duration;
  using time_point = typename Clock::time_point;

public:
  Wrapper(Myt&& move) noexcept : entry(std::move(move.entry)),
                                 delay_until(move.delay_until)
  {
    LOG(DEBUG);
  }

  Myt& operator=(Myt&& move)
  {
    std::swap(entry, move.entry);
    delay_until = move.delay_until;
    return *this;
  }

  bool operator>(const Myt& rhs) const
  {
    return delay() > rhs.delay();
  }
  bool operator<(const Myt& rhs) const
  {
    return delay() < rhs.delay();
  }
  bool operator>=(const Myt& rhs) const
  {
    return delay() >= rhs.delay();
  }
  bool operator<=(const Myt& rhs) const
  {
    return delay() <= rhs.delay();
  }

private:
  static time_point project_future(delay_type delay)
  {
    return Clock::now() + delay;
  }

  template <typename Delay>
  Wrapper(const E& entry, Delay delay)
    : entry(entry)
    , delay_until(project_future(delay))
  {
    LOG(DEBUG);
  }

  template <typename Delay>
  Wrapper(E&& entry, Delay delay)
    : entry(std::move(entry))
    , delay_until(project_future(delay))
  {
    LOG(DEBUG);
  }

  delay_type delay() const
  {
    return delay_until - Clock::now();
  }

  bool expired() const
  {
    return delay().count() <= 0;
  }

  E entry;
  time_point delay_until;
};



template <typename E, typename Clock = std::chrono::high_resolution_clock>
class DelayQueue final
{
  using Myt = DelayQueue<E, Clock>;
  using wrapper_type = Wrapper<E, Clock>;
  using Lock = std::unique_lock<std::mutex>;
  using queue_type = PriorityQueue<wrapper_type>;

public:
  DelayQueue()
    : interrupted(false)
    , nr_waiting(0)
  {
    LOG(DEBUG);
  }

  DelayQueue(const Myt& copy)
    : q(copy.q)
  {
    LOG(DEBUG);
  }

  DelayQueue(Myt&& move)
    : q(std::move(move.q))
  {
    LOG(DEBUG);
  }

  ~DelayQueue()
  {
    LOG(DEBUG);
  }

  template <typename Delay>
  void push(const E& e, Delay delay)
  {
    push(wrapper_type(e, delay));
  }

  template <typename Delay>
  void push(E&& e, Delay delay)
  {
    push(std::move(wrapper_type(std::move(e), delay)));
  }

  E pop()
  {
    LOG(DEBUG);
    Lock lock(mutex);

    for (;;) {
      if (interrupted) {
        LOG(DEBUG) << "interrupted!";
        interrupted = (nr_waiting != 0);
        throw thread_interrupted();
      }
      if (q.empty()) {
        LOG(DEBUG) << "queue is empty; waiting on available";
        ++nr_waiting;
        available.wait(lock);
        --nr_waiting;
      } else {
        auto first = &(q.top());
        if (!first->expired()) {
          LOG(DEBUG) << "first item not expired; waiting for delay";
          ++nr_waiting;
          available.wait_for(lock, first->delay());
          --nr_waiting;
        } else {
          LOG(DEBUG) << "popped";
          auto actual = q.pop();
          if (!q.empty()) {
            LOG(DEBUG) << "queue not empty; notifying";
            available.notify_all();
          }
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
    LOG(DEBUG);
    Lock lock(mutex);
    if (nr_waiting > 0) {
      interrupted = true;
      available.notify_all();
    }
  }

  Myt& operator=(const Myt& copy)
  {
    q = copy.q;
    return *this;
  }

  Myt& operator=(Myt&& move)
  {
    q = std::move(move.q);
    return *this;
  }

private:
  void push(wrapper_type delayed)
  {
    LOG(DEBUG);
    Lock lock(mutex);
    auto first = !q.empty() ? &(q.top()) : nullptr;
    q.push(std::move(delayed));
    // notify pop() that there's a sooner delay than the one it's waiting on
    if (!first || delayed < *first) {
      LOG(DEBUG) << "pushed item comes sooner than queue head; notifying";
      available.notify_all();
    }
  }

  queue_type q;
  std::mutex mutex;
  std::condition_variable available;
  bool interrupted{false};
  std::size_t nr_waiting{0};
};



}    // namespace sma
