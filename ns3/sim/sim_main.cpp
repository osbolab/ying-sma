#define _ELPP_NO_DEFAULT_LOG_FILE
#include <sma/io/log>
// Call only once per application
_INITIALIZE_EASYLOGGINGPP

#include <sma/test>

#include "wifi_simple.hpp"

#include <iostream>

int main(int argc, char** argv)
{
  _START_EASYLOGGINGPP(argc, argv);

  std::cout << "Configuring application logging from log.conf...\n";
  el::Configurations logconf("../../conf/log.conf");
  el::Loggers::reconfigureAllLoggers(logconf);
  LOG(DEBUG) << "Configuring node logging from nodelog.conf...";
  el::Configurations nodelogconf("../../conf/nodelog.conf");
  el::Loggers::setDefaultConfigurations(nodelogconf, false);

  el::Loggers::addFlag(el::LoggingFlag::LogDetailedCrashReason);
  LOG(DEBUG) << "Detailed crash reports are enabled (--logging-flags=4)";
  el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
  LOG(DEBUG) << "Colored terminal output is enabled (--logging-flags=64)";

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
