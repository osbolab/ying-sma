#pragma once

#include <iostream>
#include <cstdlib>
#include <memory>
#include <queue>
#include <functional>
#include <utility>

#include "pooled_ptr.hh"


namespace sma
{

template<class T>
class ObjectPool : public std::enable_shared_from_this<ObjectPool<T>> final
{
  template<class T>
  using Allocator = std::function<T*()>;

  template<class T>
  using Deleter = std::function<void(T*)>;


  template<class T>
  friend class pooled_ptr;

public:
  static std::shared_ptr<ObjectPool<T>>
    create(std::size_t capacity, Allocator<T> allocator, Deleter<T> deleter);

  ~ObjectPool();

  pooled_ptr<T> get();

private:
  ObjectPool(Deleter<T> deleter);

  ObjectPool(const ObjectPool<T>& copy) = delete;
  const ObjectPool<T>& operator =(const ObjectPool<T>& copy) = delete;

  void reclaim(T* ptr);

  std::queue<T*>    pool;
  const Deleter<T>  deleter;
};

#include "object_pool.tt"

}