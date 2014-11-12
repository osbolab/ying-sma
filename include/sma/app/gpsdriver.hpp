#ifndef GPS_DRIVER_H
#define GPS_DRIVER_H

#include "gpsinfo.hpp"

class GPSDriver
{
public:
  GPSinfo getGPS() const;
  bool hasGPS() const;
  void setGPS (double latitude, double longitude);

private:
  GPSinfo data;
};

#endif
