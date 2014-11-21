#include <sma/ccn/devicewithgps.hpp>
#include <sma/ccn/application.hpp>
#include <sma/context.hpp>


#include <sma/io/log>


namespace sma
{
Application::Application(Context* ctx)
  : device(std::make_unique<DeviceWithGPS>(ctx))
{
}

void Application::dispose()
{
  if (device)
    device->dispose();
}
}
