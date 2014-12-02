#pragma once

#include <sma/helper.hpp>
#include <sma/smafwd.hpp>

#include <sma/io/log>

namespace sma
{
class NeighborHelper : public Helper
{
public:
  NeighborHelper(CcnNode& node);

  virtual ~NeighborHelper() {}

  virtual void receive(MessageHeader header, Beacon msg) = 0;
};
}
