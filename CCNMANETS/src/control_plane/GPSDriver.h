#ifndef GPS_DRIVER_H
#define GPS_DRIVER_H

#include "GPSinfo.h"

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
