#include <sma/device/device.hpp>
#include <sma/device/gps_dummy_device.hpp>
#include <sma/device/gps.hpp>


namespace sma
{
gps_dummy_device::gps_dummy_device()
  : device(devinfo{"GPS", "Dummy GPS device that returns one position."})
{
}

gps_dummy_device::~gps_dummy_device() {}

gps::coord gps_dummy_device::position() const
{
  return gps::coord{30.30, 30.782};
}
}
