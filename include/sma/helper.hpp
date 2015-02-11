#pragma once

#include <sma/io/log>

namespace sma
{
struct CcnNode;

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
