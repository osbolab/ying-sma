#pragma once

#include <string>


namespace sma
{
  class device
  {
    public:
      struct devinfo {
        std::string name;
        std::string description;
      };

      virtual ~device() {}

      virtual const devinfo info() const { return device_info_; }

    protected:
      device(devinfo info)
        : device_info_(info)
      {}

      devinfo device_info_;
  };
}
