#include <sma/ccn/devicewithgps.hpp>
#include <sma/ccn/ccnapplication.hpp>
#include <sma/context.hpp>

#include <sma/io/log>


namespace sma
{
CcnApplication::CcnApplication(Context* ctx)
  : device(std::make_unique<DeviceWithGPS>(ctx))
{
}

void CcnApplication::dispose()
{
  if (device)
    device->dispose();
}
}
