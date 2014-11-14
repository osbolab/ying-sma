#pragma once


#include <sma/networkemulator.hpp>
#include <sma/datablock.hpp>
#include <sma/devicelogger.hpp>
#include <sma/typedefinition.hpp>    //temp

#include <string>

class NetworkEmulator;

class Device
{
public:
  virtual ~Device()
  {
  }
  virtual std::string getDeviceID() const = 0;
  virtual void connectToNetwork(NetworkEmulator* networkToAttach) = 0;
  virtual void leaveFromNetwork(NetworkEmulator* networkToAttach) = 0;
  virtual void receiveSignal(const DataBlock& block) = 0;
  virtual void sendSignal(const DataBlock& block) = 0;
  virtual DeviceLogger* getLoggerPtr() const = 0;
  virtual unsigned int getPowerLevel() const = 0;

  virtual void forwardRequest(ChunkID chunk) = 0;
};


