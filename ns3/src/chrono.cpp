#include <sma/chrono.hpp>

#include <ns3/simulator.h>
#include <ns3/nstime.h>

#include <chrono>
#include <ctime>


namespace sma
{
namespace chrono
{
  namespace detail
  {
    template <typename Clock>
    struct start_time_s_ {
      static start_time_s_& get()
      {
        static start_time_s_ instance;
        return instance;
      }

      typename Clock::time_point start_time{
          std::chrono::time_point_cast<typename Clock::duration>(
              std::chrono::system_clock::now())};

    private:
      start_time_s_() {}
#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotImplementedFunctions"
      start_time_s_(start_time_s_ const&);
      void operator=(start_time_s_ const&);
#pragma clang diagnostic pop
    };
  }

  system_clock::time_point system_clock::now()
  {
    auto start_time = detail::start_time_s_<system_clock>::get().start_time;
    return start_time
           + std::chrono::nanoseconds(ns3::Simulator::Now().GetNanoSeconds());
  }
}
}
