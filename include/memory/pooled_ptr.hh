#pragma once

#include <memory>
#include <iostream>


namespace sma
{

template<class T>
class pooled_ptr
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


template<class T>
pooled_ptr<T>::pooled_ptr(std::shared_ptr<ObjectPool<T>> pool, T* ptr, Deleter<T> deleter)
  : pool(pool)
{
  auto reclaimer = [pool, deleter](T* ptr)
  {
#ifdef _DEBUG
    std::cout << pool << " [~pooled_ptr]\t" << static_cast<void*>(ptr) << std::endl;
#endif
    if (pool) {
      pool->reclaim(ptr);
    } else {
      deleter(ptr);
    }
  };
  this->ptr = std::shared_ptr<T>(ptr, reclaimer);
}

template<class T>
pooled_ptr<T>::pooled_ptr(pooled_ptr<T>&& move)
  : pool(std::move(move.pool)), ptr(move.ptr)
{
  move.pool = nullptr;
  move.ptr = nullptr;
}

template<class T>
pooled_ptr<T>& pooled_ptr<T>::operator =(pooled_ptr<T>&& move)
{
  ptr = move.ptr;
  pool = std::move(move.pool);
  move.pool = nullptr;
  move.ptr = nullptr;
  return (*this);
}

template<class T>
pooled_ptr<T>::pooled_ptr(const pooled_ptr<T>& copy)
  : pool(copy.pool), ptr(copy.ptr)
{
}

template<class T>
pooled_ptr<T>& pooled_ptr<T>::operator =(const pooled_ptr<T>& copy)
{
  ptr = copy.ptr;
  pool = copy.pool;
  return (*this);
}

template<class T>
T& pooled_ptr<T>::operator *() { return *(ptr); }

template<class T>
T* pooled_ptr<T>::operator ->() { return ptr.get(); }

template<class T>
T& pooled_ptr<T>::operator [](std::size_t index) { return *((T*)ptr.get() + index); }

template<class T>
const T& pooled_ptr<T>::operator *() const { return ptr; }

template<class T>
const T* pooled_ptr<T>::operator ->() const { return ptr; }

template<class T>
const T& pooled_ptr<T>::operator [](std::size_t index) const { return *((T*)ptr.get() + index); }

}