#pragma once


#include <sma/ccn/device.hpp>
#include <string>
#include <mutex>
#include <fstream>
#include <sma/ccn/datablock.hpp>

class Device;
class DataBlock;

class NetworkEmulator
{
public: 
  virtual ~NetworkEmulator() {}
  virtual void acceptDevice (Device* device) = 0;
  virtual void dropDevice (Device* device) = 0;
  virtual void broadcastMsg() = 0;
  virtual void receiveMsg(const DataBlock & block) = 0;
//  virtual void scheduleTransmission() const = 0;
};


