#pragma once

#include <sma/device/gps.hpp>


namespace sma
{
class dummy_gps : public gps
{
public:
  dummy_gps(gps::coord pos)
    : pos(std::move(pos))
  {
  }
  virtual ~dummy_gps() {}

  virtual gps::coord position() const override { return pos; }

protected:
  gps::coord pos;
};
}
