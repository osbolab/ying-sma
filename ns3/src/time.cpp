#include <sma/time>

#include <ns3/simulator.h>
#include <ns3/nstime.h>

#include <chrono>
#include <ctime>


namespace sma
{
namespace chrono
{
  system_clock::time_point system_clock::now()
  {
    return std::chrono::time_point<system_clock, std::chrono::nanoseconds>(
            std::chrono::nanoseconds(ns3::Simulator::Now().GetNanoSeconds()));
  }

  system_clock::time_point system_clock::from_time_t(std::time_t t)
  {
    return std::chrono::time_point_cast<duration>(
        std::chrono::time_point<system_clock, std::chrono::seconds>(
          std::chrono::seconds(t)));
  }

  std::time_t system_clock::to_time_t(system_clock::time_point const& t)
  {
    return std::time_t(std::chrono::duration_cast<std::chrono::seconds>(
          t.time_since_epoch()).count());
  }
}
}

