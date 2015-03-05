#pragma once

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
