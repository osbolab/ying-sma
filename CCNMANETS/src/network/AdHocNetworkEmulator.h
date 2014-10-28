#ifndef ADHOC_NETWORK_EMULATOR_H
#define ADHOC_NETWORK_EMULATOR_H

#include "GPSinfo.h"
#include "Device.h"
#include "DeviceWithGPS.h"
#include <string>
#include <mutex>
#include <unordered_map>
#include <queue>
#include "DataBlock.h"
#include <condition_variable>
#include "NetworkEmulator.h"
#include <vector>
#include <map>

class AdHocNetworkEmulator : public NetworkEmulator
{
public:
  AdHocNetworkEmulator();
  ~AdHocNetworkEmulator();
  void acceptDevice(Device* device);
  void dropDevice(Device* device); 
  void receiveMsg(const DataBlock& block);
  void broadcastMsg();
  void showConnectedDevices() const; // for unit test

private:
//  void scheduleTransmission();
  double getDistance(Device* deviceA, Device* deviceB) const;
  void getNeighborOfDevice(Device* device, std::vector<Device*>& list) const;
  Device* getDeviceByID(std::string id) const;
  double getCommunicationRange (unsigned int power) const;
  bool allowTransmission (std::string deviceID, std::map<std::string, std::thread>& threads) const;
  bool inConflict (Device* deviceA, Device* deviceB) const;
  void unicastMsg (const DataBlock& data) const;
  void waitForBroadcast (std::map<std::string, std::thread>& threads) const;

  std::queue<DataBlock> bcastMsgFromDevices;
  std::unordered_map<std::string, Device*> deviceList;
  std::mutex m_mutex_bcastMsg;
  std::mutex m_mutex_device_list;
  std::condition_variable msgQueueCheck;
//  std::ofstream networkActivityLog;  
  static std::string LOG_FILE_DIR;
  std::thread broadcastMsgThread;
//  static float COMMUNICATION_RANGE;
  DeviceLogger* logger;
};

#endif
