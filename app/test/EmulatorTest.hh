#include "emulator/Emulator.hh"

#include "gtest/gtest.h"


TEST(Emulator, init_environment)
{
  Emulator emulation;
  emulation.initEnv(true);
}
