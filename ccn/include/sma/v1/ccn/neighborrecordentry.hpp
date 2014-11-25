#pragma once


#include <chrono>
#include <string>

class NeighborRecordEntry
{
public:
  NeighborRecordEntry(std::string id, double latitude, double longitude);
  std::string getDeviceID() const;
  std::chrono::system_clock::time_point getTimestamp() const;
  double getLatitude() const;
  double getLongitude() const;
private:
  std::string deviceID;
  double latitude;
  double longitude;
  std::chrono::system_clock::time_point timestamp;
};


