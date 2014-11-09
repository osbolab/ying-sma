#include "threadpool.hh"

#include <mutex>
#include <memory>
#include <utility>
#include <iostream>


namespace sma
{

Threadpool::Threadpool(std::size_t nr_threads, std::size_t min_capacity)
{
  if (!min_capacity)
    min_capacity = 16;

  // Round up to the next power of 2
  if (min_capacity & (min_capacity - 1)) {
    int sh = 0;
    while ((min_capacity >>= 1) != 0)
      ++sh;
    min_capacity = 1 << sh;
  }
  s = std::make_unique<shared_state>(min_capacity);

  threads.resize(nr_threads);
  while (nr_threads != 0)
    threads[--nr_threads]
        = std::thread(&Threadpool::thread_body, this, std::ref(*s));
}
Threadpool::Threadpool(std::size_t nr_threads)
  : Threadpool(nr_threads, 16)
{
}
Threadpool::~Threadpool() { join(); }

void Threadpool::thread_body(shared_state& s)
{
  std::cout << "|> start" << std::endl;
  while (!s.stop) {
    Task task{nullptr};
    {
      std::unique_lock<std::mutex> lock(s.mx);
      while (!s.count)
        s.available.wait(lock, [&s]() { return s.count; });
      task = std::move(s.tasks[s.t]);
      if (++s.t == s.tasks.size())
        s.t = 0;
      // If the load decreases enough we can dump any extra storage
      // This isn't cheap so we don't bother unless the load is quite low
      if (--s.count < (s.min_capacity / 2))
        resize(s, s.min_capacity);
    }
    if (task)
      task();
  }
}

void Threadpool::push_back(Task task)
{
  {
    auto sz = s->tasks.size();
    std::lock_guard<std::mutex> lock(s->mx);
    // If the writer has caught up with the readers, expand
    // the queue and order old elements at the head.
    // The queue prefers to grow than to shrink
    if (s->count == sz)
      sz = resize(*s, sz * 2);

    s->tasks[s->p] = std::move(task);
    if (++(s->p) == sz)
      s->p = 0;
    s->count++;
  }
  s->available.notify_one();
}

std::size_t Threadpool::resize(shared_state& s, std::size_t len)
{
  auto t_new = std::vector<Task>(len);
  ringmove(t_new, s.tasks, s.t, s.count);
  std::swap(s.tasks, t_new);
  s.t = 0;
  s.p = s.count;
  return len;
}

void Threadpool::join()
{
  {
    std::lock_guard<std::mutex>(s->mx);
    s->stop = true;
  }
  s->available.notify_all();
  for (auto& th : threads)
    th.join();
}
}
