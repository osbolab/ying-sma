#pragma once

#include <sma/util/serial.hpp>

#include <string>
#include <sstream>
#include <iomanip>


namespace sma
{
struct Location {
  TRIVIALLY_SERIALIZABLE(Location, lat, lon)

  Location(double lat, double lon)
    : lat(lat)
    , lon(lon)
  {
  }

  explicit operator std::string() const
  {
    std::ostringstream ss;
    ss << "(lat: " << std::setprecision(9) << lat
       << ", lon: " << std::setprecision(9) << lon << ")";
    return ss.str();
  }

  double lat;
  double lon;
};
}
