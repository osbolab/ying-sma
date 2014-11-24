#include <sma/io/log>
// Call only once per application
_INITIALIZE_EASYLOGGINGPP

#include <sma/test>

#include "wifi_simple.hpp"


int main(int argc, char** argv)
{
  _START_EASYLOGGINGPP(argc, argv);

  el::Loggers::reconfigureAllLoggers(
      el::ConfigurationType::Format,
      "%datetime{%m:%s.%g} %levshort (%fbase:%line) -- %msg");
  el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
