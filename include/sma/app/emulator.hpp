#pragma once


#include <sma/app/networkemulator.hpp>
#include <sma/app/device.hpp>
#include <vector>
#include <unordered_map>

class Emulator
{
public:
  Emulator();
  void initEnv(bool inBatchMode);

private:
  void runInBatch();
  void runInRealtime();
  void clearObject();
  NetworkEmulator* network;
  std::unordered_map<std::string, Device*> devices;
  Device* createDevice(std::string deviceID);
  void processCommand(std::string cmdLine);
  Device* getDeviceByID(std::string) const;
  bool running;
};


