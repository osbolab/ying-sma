#include <sma/chrono>

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
      start_time_s_(){};
      start_time_s_(start_time_s_ const&);
      void operator=(start_time_s_ const&);
    };
  }

  system_clock::time_point system_clock::now()
  {
    auto start_time = detail::start_time_s_<system_clock>::get().start_time;
    return start_time
           + std::chrono::nanoseconds(ns3::Simulator::Now().GetNanoSeconds());
  }

  system_clock::time_point system_clock::from_time_t(std::time_t t)
  {
    return std::chrono::time_point_cast<duration>(
        std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>(
            std::chrono::seconds(t)));
  }

  std::time_t system_clock::to_time_t(system_clock::time_point const& t)
  {
    return std::time_t(std::chrono::duration_cast<std::chrono::seconds>(
                           t.time_since_epoch()).count());
  }
}
}
