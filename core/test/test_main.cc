/******************************************************************************
 *
 * This file just creates a compilation unit with the tests.
 *
 *****************************************************************************/

#include "log.hh"
// Call only once per application
_INITIALIZE_EASYLOGGINGPP

//#include "bitstest.hh"
//
//#include "priorityqueuetest.hh"
//#include "delayqueuetest.hh"
//#include "bufferpooltest.hh"
//
//#include "blockingqueuetest.hh"
//#include "threadpooltest.hh"
//
//#include "messagetest.hh"
//
//#include "addresstest.hh"
//#include "nativesockettest.hh"
//#include "nativechanneltest.hh"
//
#include "threadschedulertest.hh"

#include "gtest/gtest.h"


int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  el::Loggers::reconfigureAllLoggers(
      el::ConfigurationType::Format,
      "%datetime{%m:%s.%g} %levshort [%thread] %func (%fbase:%line) %msg");

  LOG(DEBUG) << "Testing smacore";
  return RUN_ALL_TESTS();
}
