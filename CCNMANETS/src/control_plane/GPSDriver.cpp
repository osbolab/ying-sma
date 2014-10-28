#include "GPSDriver.h"
#include "GPSinfo.h"

void GPSDriver::setGPS (double lat, double lon)
{
  data.latitude = lat;
  data.longitude = lon;
}

bool GPSDriver::hasGPS() const
{
  return true;
}

GPSinfo GPSDriver::getGPS() const
{
  return data;
}
