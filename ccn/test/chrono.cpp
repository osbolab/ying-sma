#include <sma/chrono>

#include <chrono>
#include <ctime>


namespace sma
{
namespace chrono
{
  system_clock::time_point system_clock::now()
  {
    return std::chrono::time_point_cast<duration, std::chrono::system_clock>(
        std::chrono::system_clock::now());
  }
}
}
