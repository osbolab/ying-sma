/******************************************************************************
 *
 * This file just creates a compilation unit with the tests.
 *
 *****************************************************************************/

#include <sma/io/log>
#include <sma/test>
// Call only once per application
_INITIALIZE_EASYLOGGINGPP

//#include "bytebuffertest.hh"
//#include "bitstest.hh"
//
//#include "rws_mutextest.hh"
//#include "priorityqueuetest.hh"
//#include "delayqueuetest.hh"
//#include "bufferpooltest.hh"
//
//#include "blockingqueuetest.hh"
//#include "threadpooltest.hh"
//
//#include "messagetest.hh"
//#include "messengertest.hh"

//
//#include "addresstest.hh"
//#include "nativesockettest.hh"
//#include "nativechanneltest.hh"
//
//#include "threadschedulertest.hh"



int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  el::Loggers::reconfigureAllLoggers(
      el::ConfigurationType::Format,
      "%datetime{%m:%s.%g} %levshort [%thread] %func (%fbase:%line) %msg");

  LOG(DEBUG) << "Testing smacore";
  return RUN_ALL_TESTS();
}
