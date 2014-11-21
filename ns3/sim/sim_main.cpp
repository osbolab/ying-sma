#include <sma/log>
// Call only once per application
_INITIALIZE_EASYLOGGINGPP

#include <sma/test>


int main(int argc, char** argv)
{
  el::Loggers::reconfigureAllLoggers(
      el::ConfigurationType::Format,
      "%datetime{%m:%s.%g} %levshort [%thread] %func (%fbase:%line) %msg");

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
