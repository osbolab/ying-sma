#pragma once

#include <sma/app/context.hpp>
#include <sma/app/devicewithgps.hpp>

#include <memory>


namespace sma
{

class application
{
public:
  application(context ctx);

protected:
  std::unique_ptr<DeviceWithGPS> device;
};
}
