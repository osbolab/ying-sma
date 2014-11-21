#pragma once


#include <sma/app/networkemulator.hpp>
#include <sma/app/datablock.hpp>
#include <sma/app/devicelogger.hpp>
#include <sma/app/typedefinition.hpp>    //temp

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

  virtual void forwardRequest(ChunkID chunk) = 0;
};


