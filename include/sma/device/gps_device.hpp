#pragma once

#include <sma/device/device.hpp>
#include <sma/device/gps.hpp>

namespace sma
{
class gps_device : public virtual device
{
public:
  virtual ~gps_device() {}

  virtual gps::coord position() const = 0;
};
}
