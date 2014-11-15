#include <sma/threadpool.hpp>
#include <sma/log>

#include <mutex>
#include <memory>
#include <utility>


namespace sma
{

threadpool::threadpool(std::size_t nthreads, std::size_t min_capacity)
{
  LOG(DEBUG) << nthreads << " threads";
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

  threads.resize(nthreads);
  while (nthreads != 0)
    threads[--nthreads]
        = std::thread(&threadpool::thread_body, this, std::ref(*s));
}
threadpool::threadpool(std::size_t nthreads)
  : threadpool(nthreads, 16)
{
}
threadpool::~threadpool()
{
  // join() locks and notifies so don't bother
  s->stop = true;
  join();
}

void threadpool::thread_body(shared_state& s)
{
  while (!s.stop) {
    Task task{nullptr};
    {
      std::unique_lock<std::mutex> lock(s.mx);
      if (!s.stop && !s.join && !s.count)
        s.available.wait(lock, [&s]() { return s.stop || s.join || s.count; });
      if (s.stop || (!s.count && s.join))
        break;
      task = std::move(s.tasks[s.t]);
      if (++s.t == s.tasks.size())
        s.t = 0;
      --s.count;
      // If the load decreases enough we can dump any extra storage
      // This isn't cheap so we don't bother unless the load is quite low
      if (s.scaled && s.count < (s.min_capacity / 2)) {
        resize(s, s.min_capacity);
        s.scaled = false;
      }
    }
    if (s.count)
      s.available.notify_one();

    task();

    if (s.join && !s.count)
      break;
  }
}

void threadpool::push_back(Task task)
{
  {
    auto sz = s->tasks.size();
    std::lock_guard<std::mutex> lock(s->mx);
    // If the writer has caught up with the readers, expand
    // the queue and order old elements at the head.
    // The queue prefers to grow than to shrink
    if (s->count == sz) {
      sz = resize(*s, sz * 2);
      s->scaled = true;
    }

    s->tasks[s->p] = std::move(task);
    if (++(s->p) == sz)
      s->p = 0;
    s->count++;
  }
  s->available.notify_one();
}

std::size_t threadpool::resize(shared_state& s, std::size_t len)
{
  auto t_new = std::vector<Task>(len);
  ringmove(t_new, s.tasks, s.t, s.count);
  std::swap(s.tasks, t_new);
  s.t = 0;
  s.p = s.count;
  return len;
}

void threadpool::join()
{
  if (!threads.empty()) {
    {
      std::lock_guard<std::mutex>(s->mx);
      s->join= true;
      LOG(DEBUG) << "joining " << threads.size() << " workers";
    }
    s->available.notify_all();
    for (auto& th : threads)
      if (th.joinable())
        th.join();
    threads.clear();
  }
}
}
