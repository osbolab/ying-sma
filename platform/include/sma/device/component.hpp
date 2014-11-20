#pragma once

namespace sma
{
class component
{
public:
  virtual ~component() = 0;
};

inline component::~component() {}
}
