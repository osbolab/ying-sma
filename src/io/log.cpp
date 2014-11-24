#include <sma/io/log>

namespace sma
{
Logger::Logger(std::string id)
  : impl(el::Loggers::getLogger(id))
{
  i("\n\n----------------------------------- session "
    "-----------------------------------");
}
}
