#pragma once


#include <sma/gpsinfo.hpp>

class GPSDriver
{
public:
  GPSinfo getGPS() const;
  bool hasGPS() const;
  void setGPS (double latitude, double longitude);

private:
  GPSinfo data;
};


