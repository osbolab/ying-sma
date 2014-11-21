#pragma once

#include <sma/device/component.hpp>
#include <sma/device/gps.hpp>


namespace sma
{
class gps_component : public component
{
public:
  virtual ~gps_component() {}

  virtual gps::coord position() const = 0;
};
}
