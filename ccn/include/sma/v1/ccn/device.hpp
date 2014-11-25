#pragma once


#include <sma/ccn/networkemulator.hpp>
#include <sma/ccn/datablock.hpp>
#include <sma/ccn/devicelogger.hpp>

#include <string>

class NetworkEmulator;

class Device
{
public:
  virtual ~Device()
  {
  }
  virtual std::string getDeviceID() const = 0;
  virtual void receiveSignal(const DataBlock& block) = 0;
  virtual void sendSignal(const DataBlock& block) = 0;
  virtual DeviceLogger* getLoggerPtr() const = 0;

  virtual void forwardRequest(std::string chunk) = 0;
};


