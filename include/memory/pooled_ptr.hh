#pragma once

#include <memory>
#include <iostream>


namespace sma
{

template<class T>
class pooled_ptr final
{
  template<class T>
  using Deleter = std::function<void(T*)>;


  template<class T>
  friend class ObjectPool;

public:
  pooled_ptr(pooled_ptr<T>&& move);
  pooled_ptr<T>& operator =(pooled_ptr<T>&& move);

  pooled_ptr(const pooled_ptr<T>& copy);
  pooled_ptr<T>& operator =(const pooled_ptr<T>& copy);

  T& operator *();
  T* operator ->();
  T& operator [](std::size_t index);
  const T& operator *() const;
  const T* operator ->() const;
  const T& operator [](std::size_t index) const;

private:
  pooled_ptr(std::shared_ptr<ObjectPool<T>> pool, T* ptr, Deleter<T> deleter);

  std::shared_ptr<ObjectPool<T>>  pool;
  std::shared_ptr<T>              ptr;
};

#include "pooled_ptr.tt"

}