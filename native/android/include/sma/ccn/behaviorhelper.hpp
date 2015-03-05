#pragma once

#include <sma/helper.hpp>
#include <sma/smafwd.hpp>
#include <chrono>

namespace sma
{
class CcnNode;

class BehaviorHelper : public Helper
{
public:
  BehaviorHelper (CcnNode& node);

  virtual ~BehaviorHelper() {}

  virtual void behave() = 0;
};
}
