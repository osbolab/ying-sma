#pragma once

#include <utility>
#include <future>
#include <type_traits>



namespace sma
{

template<typename T>
class Future;

class UntypedFuture
{
public:
  using pointer = std::unique_ptr<UntypedFuture>;

  virtual ~UntypedFuture() = 0;

  template<typename T>
  std::future<T> unwrap()
  {
    return std::move(dynamic_cast<Future<T>*>(this)->f);
  }
};

inline UntypedFuture::~UntypedFuture() {}



template<typename T>
class Future final : public UntypedFuture
{
  friend class UntypedFuture;

  using Myt = Future<T>;

  Future(Myt&& move)
    : f(std::move(move.f))
  {
  }

  Future(const Myt& copy) = delete;
  Myt& operator =(const Myt& copy) = delete;

public:
  Future(std::future<T>&& move)
    : f(std::move(move))
  {
  }

  Myt& operator =(Myt&& move)
  {
    std::swap(f, move.f);
    return *this;
  }

private:
  std::future<T> f;
};



template<typename T>
class Future<T&> final : public UntypedFuture
{
  friend class UntypedFuture;

  using Myt = Future<T&>;

  Future(Myt&& move)
    : f(std::move(move.f))
  {
  }

  Future(const Myt& copy) = delete;
  Myt& operator =(const Myt& copy) = delete;

public:
  Future(std::future<T&>&& move)
    : f(std::move(move))
  {
  }

  Myt& operator =(Myt&& move)
  {
    std::swap(f, move.f);
    return *this;
  }

private:
  std::future<T&> f;
};




template<>
class Future<void> final : public UntypedFuture
{
  friend class UntypedFuture;

  using Myt = Future<void>;

  Future(Myt&& move)
    : f(std::move(move.f))
  {
  }

  Future(const Myt& copy) = delete;
  Myt& operator =(const Myt& copy) = delete;

public:
  Future(std::future<void>&& move)
    : f(std::move(move))
  {
  }

  Myt& operator =(Myt&& move)
  {
    std::swap(f, move.f);
    return *this;
  }


private:
  std::future<void> f;
};

}