#pragma once

#include <sma/app/networkemulator.hpp>
#include <sma/app/device.hpp>
#include <sma/app/context.hpp>

#include <vector>
#include <unordered_map>


class Emulator
{
public:
  Emulator(sma::context ctx);
  void initEnv(bool inBatchMode = false);

private:
  void runInBatch();
  void runInRealtime();
  void clearObject();
  sma::context ctx;
  NetworkEmulator* network;
  std::unordered_map<std::string, Device*> devices;
  Device* createDevice(std::string deviceID);
  void processCommand(std::string cmdLine);
  Device* getDeviceByID(std::string) const;
  bool running;
};
