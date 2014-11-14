#pragma once


#include "networkemulator.hpp"
#include "datablock.hpp"
#include "devicelogger.hpp"
#include "typedefinition.hpp"    //temp

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


