#include <sma/app/application.hpp>
#include <sma/app/context.hpp>

#include <sma/app/devicewithgps.hpp>

#include <sma/log>


namespace sma
{
application::application(context ctx)
  : device(std::make_unique<DeviceWithGPS>(ctx))
{
  LOG(DEBUG) << "Node " << ctx.node_id;
}

void application::dispose()
{
  if (device)
    device->dispose();
}
}
