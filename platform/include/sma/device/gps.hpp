#pragma once

namespace sma
{
class gps final
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

private:
  gps();
};
}
