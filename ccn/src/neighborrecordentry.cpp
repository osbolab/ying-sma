#include <sma/ccn/neighborrecordentry.hpp>
#include <string>

NeighborRecordEntry::NeighborRecordEntry(std::string id, double lat, double lon):deviceID(id),
                                                                                latitude(lat),
                                                                                longitude(lon)
{
  timestamp = std::chrono::system_clock::now();  
}

std::string NeighborRecordEntry::getDeviceID() const
{
  return deviceID;
}

std::chrono::system_clock::time_point NeighborRecordEntry::getTimestamp() const
{
  return timestamp;
}

double NeighborRecordEntry::getLatitude() const
{
  return latitude;
} 

double NeighborRecordEntry::getLongitude() const
{
  return longitude;
}
