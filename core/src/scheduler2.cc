#include "scheduler2.hh"


namespace sma
{

template<typename F,
         typename... Args,
         typename Delay>
auto Scheduler::schedule(Delay delay, F f, Args... args)
-> std::shared_ptr<Task<std::result_of<F(Args)>>> {
  using return_type = typename std::result_of<F(Args...)>::type;


}

}