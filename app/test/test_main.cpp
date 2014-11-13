/******************************************************************************
 *
 * This file just creates a compilation unit with the tests.
 *
 *****************************************************************************/

#include <sma/log.hpp>
// Call only once per application
_INITIALIZE_EASYLOGGINGPP

// Include tests here

#include <sma/test.hpp>


int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  el::Loggers::reconfigureAllLoggers(
      el::ConfigurationType::Format,
      "%datetime{%m:%s.%g} %levshort [%thread] %func (%fbase:%line) %msg");

  return RUN_ALL_TESTS();
}
