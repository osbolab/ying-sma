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

  template<typename T>
  static pointer wrap(std::future<T>&& fut)
  {
    return pointer {
      dynamic_cast<UntypedFuture*>(
      new Future<T> { std::move(fut) }
      )
    };
  }

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

  Future(std::future<T>&& move)
    : f(std::move(move))
  {
  }

  Future(Myt&& move)
    : f(std::move(move.f))
  {
  }

  Future(const Myt& copy) = delete;
  Myt& operator =(const Myt& copy) = delete;

public:
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

  Future(std::future<T&>&& move)
    : f(std::move(move))
  {
  }

  Future(Myt&& move)
    : f(std::move(move.f))
  {
  }

  Future(const Myt& copy) = delete;
  Myt& operator =(const Myt& copy) = delete;

public:
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

  Future(std::future<void>&& move)
    : f(std::move(move))
  {
  }

  Future(Myt&& move)
    : f(std::move(move.f))
  {
  }

  Future(const Myt& copy) = delete;
  Myt& operator =(const Myt& copy) = delete;

public:
  Myt& operator =(Myt&& move)
  {
    std::swap(f, move.f);
    return *this;
  }


private:
  std::future<void> f;
};

}