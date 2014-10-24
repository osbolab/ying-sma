#pragma once

#include <mutex>
#include <condition_variable>
#include <deque>
#include <utility>

#include "concurrent/BlockingSource.hh"


namespace sma
{

template<typename T>
class BlockingQueue : public BlockingSource<T>
{
public:
  //! Create an unbounded queue.
  BlockingQueue();
  //! Create a bounded queue.
  BlockingQueue(std::size_t capacity);
  BlockingQueue(const BlockingQueue<T>& copy);
  BlockingQueue(BlockingQueue<T>&& move) noexcept;

  BlockingQueue<T>& operator =(const BlockingQueue<T>& copy);
  BlockingQueue<T>& operator =(BlockingQueue<T>&& move) noexcept;

  /*! Place an item at the end of the queue if the queue is not full.
   * \return \a true if the item was placed in the queue.
   */
  bool offer(const T& item);

  /*! Retrieve the next available item if one is available.
   * \param[out] item Contains the item retrieved if \a poll returned \a true.
   * \return \a true if an item was retrieved or \a false if none is available.
   */
  bool poll(T& item) override;

  //! Retrieve the next available item, blocking until one is available.
  T take() override;

private:
  std::deque<T>             items;
  std::size_t               capacity;
  std::mutex                mutex;
  std::condition_variable   itemAvailable;
};



template<typename T>
BlockingQueue<T>::BlockingQueue()
  : capacity(-1)
{
}

template<typename T>
BlockingQueue<T>::BlockingQueue(std::size_t capacity)
  : capacity(capacity)
{
}

template<typename T>
BlockingQueue<T>::BlockingQueue(const BlockingQueue<T>& copy)
  : items(copy.items),
    capacity(copy.capacity)
{
}

template<typename T>
BlockingQueue<T>::BlockingQueue(BlockingQueue<T>&& move)
{
  std::swap(items, move.items);
  capacity = move.capacity;
}

template<typename T>
BlockingQueue<T>& BlockingQueue<T>::operator =(const BlockingQueue<T>& copy)
{
  items = std::deque<T>(copy.items);
  capacity = copy.capacity;
  return *this;
}

template<typename T>
BlockingQueue<T>& BlockingQueue<T>::operator =(BlockingQueue<T>&& move)
{
  std::swap(items, move.items);
  capacity = move.capacity;
  return *this;
}

template<typename T>
bool BlockingQueue<T>::offer(const T& item)
{
  {
    std::unique_lock<std::mutex> lock(mutex);
    if (capacity >= 0 && items.size() >= capacity) return false;
    items.push_front(std::move(item));
  }
  itemAvailable.notify_one();
  return true;
}

template<typename T>
bool BlockingQueue<T>::poll(T& item)
{
  std::unique_lock<std::mutex> lock(mutex);

  if (items.empty()) return false;
  item = std::move(items.back());
  items.pop_back();
  return true;
}

template<typename T>
T BlockingQueue<T>::take()
{
  std::unique_lock<std::mutex> lock(mutex);

  itemAvailable.wait(lock, [&] { return !items.empty(); });
  T item(std::move(items.back()));
  items.pop_back();
  return item;
}

}