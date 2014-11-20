#pragma once

#include <sma/device/component.hpp>


namespace sma
{
class gps : public component
{
public:
  struct coord {
    static double dist(coord const& lhs, coord const& rhs);
    static double dist_sq(coord const& lhs, coord const& rhs);

    coord();
    coord(double lon, double lat);

    double operator-(coord const& rhs);
    bool operator==(coord const& rhs);
    bool operator!=(coord const& rhs);

    double lon;
    double lat;
  };


  virtual ~gps() {}

  virtual coord position() const = 0;
};
}
