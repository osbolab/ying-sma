#pragma once

#include <sma/ccn/devicewithgps.hpp>

#include <memory>


namespace sma
{

struct Context;

class Application
{
public:
  Application(Context* ctx);

  void dispose();

protected:
  std::unique_ptr<DeviceWithGPS> device;
};
}
