#include <sma/app/application.hpp>
#include <sma/context.hpp>

#include <sma/app/devicewithgps.hpp>

#include <sma/io/log>


namespace sma
{
application::application(context ctx)
  : device(std::make_unique<DeviceWithGPS>(ctx))
{
}

void application::dispose()
{
  if (device)
    device->dispose();
}
}
