#include <sma/chrono.hpp>
#include <sma/io/log>

#include <ctime>
#include <string>

namespace sma
{
namespace chrono
{
  std::string system_clock::strftime(system_clock::time_point const& tp,
                                     const char* format)
  {
    auto tm = system_clock::to_time_t(tp);
    char tmstr[32];
    auto size = std::strftime(tmstr, sizeof tmstr, format, std::gmtime(&tm));
    if (size)
      return std::string(tmstr);
    else
      LOG(ERROR) << "std::strftime overflow converting time_point; "
                    "resulting string is longer than " << (sizeof tmstr)
                 << " characters.";
    return "";
  }

  std::string system_clock::utcstrtime(system_clock::time_point const& tp)
  {
    return strftime(tp, "%c %Z");
  }
}
}
