#pragma once
#pragma GCC diagnostic ignored "-Wextern-c-compat"

#include <sma/io/log.hpp>

namespace sma
{
class CcnNode;

class Helper
{
public:
  Helper(CcnNode& node);
  virtual ~Helper() {}

protected:
  CcnNode& node;
  Logger log;
};
}
