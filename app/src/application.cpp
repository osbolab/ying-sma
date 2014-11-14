#include <sma/app/application.hpp>
#include <sma/app/context.hpp>

#include <sma/app/devicewithgps.hpp>

#include <sma/log.hpp>


namespace sma
{
application::application(context ctx)
  : device(std::make_unique<DeviceWithGPS>(ctx))
{
  LOG(DEBUG) << "Device created - application running";
}
}
