#include <sma/device/gps.hpp>
#include <cmath>


namespace sma
{
double gps::coord::distance(const gps::coord& to)
{
  static const double DEG2RAD = 0.01745329251;

  double th1 = lat * DEG2RAD;
  double th2 = to.lat * DEG2RAD;
  double ph2 = to.lon;
  double ph1 = (lon - to.lon) * DEG2RAD;

  double dz = sin(th1) - sin(th2);
  double dx = cos(ph1) * cos(th1) - cos(th2);
  double dy = sin(ph1) * cos(th1);

  return asin(sqrt(dx * dx + dy * dy + dz * dz) * 0.5) * 12742;
}
}
