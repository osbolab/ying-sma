#include <sma/app/emulator.hpp>

#include <sma/test.hpp>


namespace sma
{

TEST(app_emulator, it_runs)
{
  Emulator em;
  em.initEnv();
}

}
