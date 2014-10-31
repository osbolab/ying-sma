#ifndef DEVICE_H
#define DEVICE_H

#include "NetworkEmulator.h"
#include <string>
#include "DataBlock.h"
#include "DeviceLogger.h"
#include "TypeDefinition.h" //temp

class NetworkEmulator;

class Device
{
public:
  virtual ~Device() {}
  virtual std::string getDeviceID() const = 0;
  virtual void connectToNetwork(NetworkEmulator* networkToAttach) = 0;
  virtual void leaveFromNetwork(NetworkEmulator* networkToAttach) = 0;
  virtual void receiveSignal (const DataBlock& block) = 0;
  virtual void sendSignal (const DataBlock& block) = 0;
  virtual DeviceLogger* getLoggerPtr() const = 0;
  virtual unsigned int getPowerLevel() const = 0;

  virtual void forwardRequest (ChunkID chunk) = 0;
};

#endif
