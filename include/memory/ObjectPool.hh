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
class ObjectPool : public std::enable_shared_from_this<ObjectPool<T>>
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


template<class T>
std::shared_ptr<ObjectPool<T>> 
ObjectPool<T>::create(std::size_t capacity, Allocator<T> allocator, Deleter<T> deleter)
{
  auto instance = std::shared_ptr<ObjectPool<T>>(new ObjectPool<T>(deleter));
  while (capacity-- > 0) {
    auto ptr = allocator();
    instance->pool.push(ptr);
  }
  return instance;
}


template<class T>
ObjectPool<T>::ObjectPool(Deleter<T> deleter) : deleter(deleter)
{
#ifdef _DEBUG
  std::cout << static_cast<void*>(this) << " [ObjectPool]" << std::endl;
#endif
}


template<class T>
ObjectPool<T>::~ObjectPool()
{
  while (!pool.empty()) {
    deleter(pool.front());
    pool.pop();
  }
#ifdef _DEBUG
  std::cout << static_cast<void*>(this) << " [~ObjectPool]" << std::endl;
#endif
}


template<class T>
pooled_ptr<T> ObjectPool<T>::get()
{
  if (pool.empty()) {
    std::cerr << "Object pool exhausted" << std::endl << std::flush;
    std::exit(1);
  }

  T* ptr = pool.front();
  pool.pop();
#ifdef _DEBUG
  std::cout << static_cast<void*>(this) << " [get]\t\t" << static_cast<void*>(ptr) << std::endl;
#endif
  return pooled_ptr<T>(this->shared_from_this(), ptr, deleter);
}


template<class T>
void ObjectPool<T>::reclaim(T* ptr)
{
#ifdef _DEBUG
    std::cout << static_cast<void*>(this) << " [reclaim]\t" << static_cast<void*>(ptr) << std::endl;
#endif
    pool.push(ptr);
}

}