#include <sma/device/gps.hpp>

#include <cassert>
#include <cmath>


namespace sma
{
gps::coord::coord()
  : lon(0.0)
  , lat(0.0)
{
}
gps::coord::coord(double lon, double lat)
  : lon(lon)
  , lat(lat)
{
}

double gps::coord::dist(coord const& lhs, coord const& rhs)
{
  static double const DEG2RAD = 0.01745329251;

  double const th1 = lhs.lat * DEG2RAD;
  double const th2 = rhs.lat * DEG2RAD;
  double const ph2 = rhs.lon;
  double const ph1 = (lhs.lon - rhs.lon) * DEG2RAD;

  double const dz = sin(th1) - sin(th2);
  double const dx = cos(ph1) * cos(th1) - cos(th2);
  double const dy = sin(ph1) * cos(th1);

  return asin(sqrt(dx * dx + dy * dy + dz * dz) * 0.5) * 12742;
}

double gps::coord::operator-(coord const& b)
{
  return dist(*this, b);
}

bool double_eq(double a, double b)
{
  static long long const EPSILON = 1;

  assert(sizeof(double) == sizeof(long long));
  if (a == b) return true;
  long long d = std::abs(*(long long*)&a - *(long long*)&b);
  return (d <= EPSILON);
}

bool gps::coord::operator==(coord const& rhs)
{
  return double_eq(lon, rhs.lon) && double_eq(lat, rhs.lat);
}
bool gps::coord::operator!=(coord const& rhs)
{
  return !(*this == rhs);
}
}
