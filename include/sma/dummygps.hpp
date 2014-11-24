#pragma once

#include <sma/gps.hpp>
#include <sma/gpscomponent.hpp>

namespace sma
{
class DummyGps : public GpsComponent
{
public:
  DummyGps(GPS::Coord pos)
    : pos(std::move(pos))
  {
  }
  virtual ~DummyGps() {}

  virtual GPS::Coord position() const override { return pos; }

protected:
  GPS::Coord pos;
};
}
