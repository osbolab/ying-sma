#include <sma/device/gps_device.hpp>
#include <sma/device/gps.hpp>

namespace sma
{
class gps_dummy_device : public gps_device
{
public:
  gps_dummy_device();
  virtual ~gps_dummy_device();
  virtual gps::coord position() const override;
};
}
