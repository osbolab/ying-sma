#pragma once

#include <sma/app/devicewithgps.hpp>

#include <memory>


namespace sma
{

struct context;

class application
{
public:
  application(context ctx);

  void dispose();

protected:
  std::unique_ptr<DeviceWithGPS> device;
};
}
