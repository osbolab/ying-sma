#include <sma/io/log>
// Call only once per application
_INITIALIZE_EASYLOGGINGPP

#include <sma/test>

#include "wifi_simple.hpp"

#include <iostream>

int main(int argc, char** argv)
{
  _START_EASYLOGGINGPP(argc, argv);

  std::cout << "Configuring logging from log.conf...\n";
  el::Configurations logconf("log.conf");
  el::Loggers::reconfigureAllLoggers(logconf);
  el::Loggers::addFlag(el::LoggingFlag::LogDetailedCrashReason);
  LOG(INFO) << "Detailed crash reports are enabled (--logging-flags=4)";
  el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
  LOG(INFO) << "Colored terminal output is enabled (--logging-flags=64)";

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
