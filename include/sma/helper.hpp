#pragma once

#include <sma/smafwd.hpp>
#include <sma/io/log>

namespace sma
{
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
