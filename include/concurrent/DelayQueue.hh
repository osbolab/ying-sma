#pragma once

#include <queue>
#include <chrono>
#include <mutex>
#include <condition_variable>


namespace sma
{

template<typename T, typename Clock>
class Delayed;


template<typename E,
         typename Clock = std::chrono::high_resolution_clock>
class DelayQueue
{
  using MyT = DelayQueue<E, Clock>;
  using delayed_t = Delayed<E, Clock>;
  using lock_t = std::unique_lock<std::mutex>;

public:
  DelayQueue()
  {
  }

  DelayQueue(const MyT& copy)
    : q(copy.q)
  {
  }

  DelayQueue(MyT&& move)
    : q(std::move(move.q))
  {
  }

  template<typename DelayT>
  void push(const E& e, DelayT delay)
  {
    push(delayed_t(e, delay));
  }

  template<typename DelayT>
  void push(E&& e, DelayT delay)
  {
    push(delayed_t(std::move(e), delay));
  }

  E take()
  {
    lock_t lock(mutex);

    for (;;) {
      if (q.empty()) available.wait(lock);
      else {
        delayed_t* first = &(q.top());
        if (first->delay().count() > 0) {
          available.wait_for(lock, first->delay());
        } else {
          auto actual = std::move(q.top());
          q.pop();
          if (!q.empty()) available.notify_all();
          return std::move(actual.item);
        }
      }
    }
  }

  MyT& operator =(const MyT& copy)
  {
    q = copy.q;
    return *this;
  }

  MyT& operator =(MyT&& move)
  {
    q = std::move(move.q);
    return *this;
  }

private:
  void push(delayed_t&& delayed)
  {
    lock_t lock(mutex);
    const delayed_t* first = !q.empty() ? &q.top() : nullptr;
    q.push(std::move(delayed));
    // notify take() that there's a sooner delay than the one it's waiting on
    if (!first || delayed < *first) available.notify_all();
  }


  std::priority_queue<delayed_t, std::deque<delayed_t>, std::greater<delayed_t>> q;
  std::mutex mutex;
  std::condition_variable available;
};



template<typename T, typename Clock>
class Delayed final
{
  using MyT = Delayed<T, Clock>;

  template<typename T, typename Clock>
  friend class DelayQueue;

  using delay_t = typename Clock::duration;

public:
  Delayed(MyT&& move)
    : item(std::move(move.item)), delay_until(move.delay_until)
  {
  }

  MyT& operator =(MyT&& move)
  {
    std::swap(item, move.item);
    delay_until = move.delay_until;
    return *this;
  }

  bool operator >(const MyT& rhs) const { return delay() > rhs.delay(); }
  bool operator <(const MyT& rhs) const { return delay() < rhs.delay(); }
  bool operator >=(const MyT& rhs) const { return delay() >= rhs.delay(); }
  bool operator <=(const MyT& rhs) const { return delay() <= rhs.delay(); }

private:
  static typename Clock::time_point project_future(delay_t delay)
  {
    return Clock::now() + delay;
  }

  template<typename DelayT>
  Delayed(const T& item, DelayT delay)
    :item(item), delay_until(project_future(delay))
  {
  }

  template<typename DelayT>
  Delayed(T&& item, DelayT delay)
    : item(std::move(item)), delay_until(project_future(delay))
  {
  }

  delay_t delay() const
  {
    return delay_until - Clock::now();
  }

  Delayed(const MyT& copy) = delete;
  MyT& operator =(const MyT& copy) = delete;


  T item;
  typename Clock::time_point delay_until;
};

}