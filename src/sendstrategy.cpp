#include <sma/sendstrategy.hpp>
#include <sma/linklayer.hpp>

namespace sma
{
  SendStrategy::SendStrategy(LinkLayer& llayer)
    : llayer(&llayer)
  {
    llayer.send_strategy(*this);
  }
}
