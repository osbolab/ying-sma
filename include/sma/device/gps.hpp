#pragma once


namespace sma
{
class gps
{
public:
  struct coord {
    double lon;
    double lat;

    double distance(const coord& to);
  };
  struct dms_coord {
    bool northeast;
    double degree;
    double minute;
    double second;
  };
};
}
