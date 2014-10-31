#include "schedule/Task.hh"

namespace sma
{

template<typename F, typename... Args>
static Task::f_wrapper Task::wrap(F&& f, Args&& ... args)
{
  // Deduce the type of the function created by f(args) so we can pass it to
  // bind without specifying it
  using return_type = typename std::result_of<F(Args...)>::type;

  // Create a late binding to f(args) in a copyable pointer
  auto binding = std::make_shared<return_type>(
                   std::bind(std::forward<F>(f), std::forward<Args>(args)...);
                 );

  // Move capture comes in C++14; until then we eat the ref++/ref-- on the shared_ptr
  // The closure copies the pointer to the binding and deduces the type. Calling the
  // void(*)() dereferences the pointer the binding is equivalent to f(args).
  return f_wrapper([binding]() { (*binding)(); });
}

}