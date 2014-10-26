#pragma once

#include "concurrent/Future.hh"

#include <cstdlib>
#include <memory>
#include <thread>
#include <functional>


namespace sma
{

class Task
{
public:
  using f_wrapper = std::function<void()>;




  struct Target {
    friend class Task;

    Target(Target&& move)
      : f(std::move(move.f)), result(std::move(move.result))
    {
    }

    Target& operator =(Target&& move)
    {
      std::swap(f, move.f);
      std::swap(result, move.result);
      return *this;
    }

  private:
    Target(f_wrapper f, UntypedFuture::pointer result)
      : f(std::move(f)), result(std::move(result))
    {
    }

    Target& operator =(const Target& copy) = delete;

  public:
    f_wrapper f;
    UntypedFuture::pointer result;
  };





  template<typename F, typename... Args>
  static Target target(F&& f, Args&& ... args)
  {
    // Deduce the type of the function created by f(args) so we can pass it to
    // bind without specifying it
    using return_type = typename std::result_of<F(Args...)>::type;

    // Bind f(args) in an asynchronous task runner in a copyable pointer
    auto binding =
      std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
      );

    // Move capture comes in C++14; until then we eat the ref++/ref-- on the shared_ptr
    // The closure copies the pointer to the binding and deduces the type. Calling the
    // void(*)() dereferences the pointer the binding is equivalent to f(args).
    // We also attach the future so the task result can be retrieved.
    return Target(
    f_wrapper([binding]() { (*binding)(); }),
    wrap(std::move(binding->get_future()))
           );
  }



  virtual ~Task() {}

  //! \return \c true if the task is not scheduled to run.
  virtual bool done() = 0;

  //! \return \c true if the task can be cancelled or \c false if it isn't scheduled.
  virtual bool cancellable() = 0;


private:
  template<typename T>
  static UntypedFuture::pointer wrap(std::future<T>&& fut)
  {
    return UntypedFuture::pointer(
             dynamic_cast<UntypedFuture*>(
               new Future<T>(std::move(fut))
             )
           );
  }
};

}