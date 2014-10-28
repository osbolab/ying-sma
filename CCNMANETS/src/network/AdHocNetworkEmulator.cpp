#include "AdHocNetworkEmulator.h"
#include <mutex>
#include <cassert>
#include "GPSinfo.h"
#include <iostream>
#include "Device.h"
#include <string>
#include <cmath>
#include <cstdlib>
#include <condition_variable>
#include <thread>
#include <chrono>
#include "DeviceLogger.h"
#include <sstream>
#include <map>

#define R 6371
#define TO_RAD (3.1415926536 / 180)

std::string AdHocNetworkEmulator::LOG_FILE_DIR = "./log/";
//float AdHocNetworkEmulator::COMMUNICATION_RANGE = 0.1;  //km

AdHocNetworkEmulator::AdHocNetworkEmulator()
{
  std::string logFileName = LOG_FILE_DIR + "network_activity.log";
//  networkActivityLog.open(logFileName);
  logger = new DeviceLogger (logFileName);
  broadcastMsgThread = std::thread (&AdHocNetworkEmulator::broadcastMsg, this);
}

AdHocNetworkEmulator::~AdHocNetworkEmulator()
{
//  networkActivityLog.close();
  broadcastMsgThread.detach();
  logger->close();
  delete logger;
//  delete broadcastMsgThread;
}

void AdHocNetworkEmulator::acceptDevice(Device* device)
{
//  std::cout << "Adding device... " << device->getDeviceID() << std::endl;
  std::unique_lock<std::mutex> lock_list (m_mutex_device_list, std::defer_lock); // synchronization
  lock_list.lock();
  std::string deviceID = device->getDeviceID();
  deviceList.insert(std::make_pair(deviceID, device));
  lock_list.unlock();
//  showConnectedDevices();
}

void AdHocNetworkEmulator::dropDevice(Device* device)
{
//  std::cout << "Dropping device..." << device->getDeviceID() << std::endl;
  std::unique_lock<std::mutex> lock_list (m_mutex_device_list, std::defer_lock);
  lock_list.lock();
  std::string deviceID = device->getDeviceID();
  deviceList.erase(deviceList.find(deviceID));
  lock_list.unlock();
//  showConnectedDevices();
}

void AdHocNetworkEmulator::showConnectedDevices() const
{
  std::unordered_map<std::string, Device*>::const_iterator iter = deviceList.begin();
  while (iter != deviceList.end())
  {
    DeviceWithGPS* devicePtr = (DeviceWithGPS*) iter->second;
    assert (devicePtr->getDeviceID() == iter->first);
    std::cout << "Device ID: " << iter->first << std::endl;
    GPSinfo gpsData = devicePtr->getGPS();
    std::cout << "GPS info: " << gpsData.latitude << ", " << gpsData.longitude << std::endl;
    std::unordered_map<std::string, Device*>::const_iterator iterAfter = deviceList.begin();
    while (iterAfter != deviceList.end())
    {
      if (iterAfter != iter)
      {
        std::cout << "\tDevice ID: " << iterAfter->first << std::endl;
        DeviceWithGPS* deviceAfterPtr = (DeviceWithGPS*) iterAfter->second;
        std::cout << "\tGPS Info: " << deviceAfterPtr->getGPS().latitude << ", "
                                  << deviceAfterPtr->getGPS().longitude << std::endl;
        std::cout << "The distance to " << iter->first << "is "
                << getDistance (devicePtr, deviceAfterPtr) << std::endl;
      }
      iterAfter++;
    }
    iter++;
    std::cout << std::endl;
  }
}

void AdHocNetworkEmulator::broadcastMsg() // if queue is not empty, broadcast
{
  std::unique_lock<std::mutex> lock_bcastMsg (m_mutex_bcastMsg, std::defer_lock);
  std::map <std::string, std::thread> ongoingTransmissionThreads;
  std::vector<DataBlock> swapQueue;
  while (true)
  {
    lock_bcastMsg.lock();
    while (bcastMsgFromDevices.empty())
    {
      msgQueueCheck.wait(lock_bcastMsg);
    }

    while (!bcastMsgFromDevices.empty())
    {
      DataBlock data = bcastMsgFromDevices.front(); 
      bcastMsgFromDevices.pop();  
      std::ostringstream oss;
      if (this->allowTransmission (data.getSrcDeviceID(), ongoingTransmissionThreads))
      {
        ongoingTransmissionThreads.insert (std::make_pair(data.getSrcDeviceID(), 
                                           std::thread(&AdHocNetworkEmulator::unicastMsg, this, data))
                                          );
        oss << "New unicast task for chunk " << data.getChunkID() << " from device " << data.getSrcDeviceID() << std::endl;
        logger-> log (oss.str());
      }
      else
      {
        swapQueue.push_back (data); //add to the virtual swap vector
        oss << "Unicast chunk " << data.getChunkID() << " suspended because of conflict" << std::endl;
        logger-> log (oss.str());
      }
    } 
    lock_bcastMsg.unlock();
 
    //join the unicast threads;
    this->waitForBroadcast (ongoingTransmissionThreads);
    //move block in swap vector to the msgQueue.

    lock_bcastMsg.lock();
    std::vector<DataBlock>::iterator swap_iter = swapQueue.begin();
    while (swap_iter != swapQueue.end())
    {
      bcastMsgFromDevices.push (*swap_iter);
      swap_iter++;
    }
    lock_bcastMsg.unlock();

    // empty swap vector and ongoing transmissionThreads
    swapQueue.clear(); 
    ongoingTransmissionThreads.clear();
  }
}

void AdHocNetworkEmulator::waitForBroadcast (std::map<std::string, std::thread>& threads) const
{
  std::map <std::string, std::thread>::iterator iter = threads.begin();
  while (iter != threads.end())
  {
    (iter->second).join();
    iter++;
  }     
  logger->log ("Broadcast completed.\n");
}

void AdHocNetworkEmulator::unicastMsg (const DataBlock& data) const
{
  Device* devicePtr = getDeviceByID(data.getSrcDeviceID());   
  std::vector<Device*> neighborList;
  getNeighborOfDevice(devicePtr, neighborList);
  std::vector<Device*>::iterator iter = neighborList.begin();
  while (iter != neighborList.end())
  {
    (*iter)->receiveSignal(data);
    iter++;
  }
}

bool AdHocNetworkEmulator::allowTransmission (std::string deviceID, std::map <std::string, std::thread>& threads) const
{
  bool result = true;
  std::map <std::string, std::thread>::iterator iter = threads.begin();
  while (iter != threads.end())
  {
    if (this->inConflict (this->getDeviceByID(deviceID), this->getDeviceByID(iter->first)))
    {
      result = false;
      break; 
    }
    iter++;
  }
  return result;
}

bool AdHocNetworkEmulator::inConflict (Device* deviceA, Device* deviceB) const  //need defining the conflict further.
{
  return ( this->getCommunicationRange(deviceA->getPowerLevel())
         + this->getCommunicationRange(deviceB->getPowerLevel())
         >= this->getDistance(deviceA, deviceB)
         );
}

double AdHocNetworkEmulator::getDistance(Device* deviceA, Device* deviceB) const
{
  double th1 = ((DeviceWithGPS*) deviceA)->getGPS().latitude;
  double ph1 = ((DeviceWithGPS*) deviceA)->getGPS().longitude;
  double th2 = ((DeviceWithGPS*) deviceB)->getGPS().latitude;
  double ph2 = ((DeviceWithGPS*) deviceB)->getGPS().longitude;

  double dx, dy, dz;
  ph1 -= ph2;
  ph1 *= TO_RAD, th1 *= TO_RAD, th2 *= TO_RAD;
 
  dz = sin(th1) - sin(th2);
  dx = cos(ph1) * cos(th1) - cos(th2);
  dy = sin(ph1) * cos(th1);

  return asin(sqrt(dx * dx + dy * dy + dz * dz) / 2) * 2 * R;
}

void AdHocNetworkEmulator::receiveMsg (const DataBlock& block)
{
  std::unique_lock<std::mutex> lock_bcastMsg (m_mutex_bcastMsg, std::defer_lock);
  lock_bcastMsg.lock();
  std::ostringstream oss;
  oss << "Receiving block from " << block.getSrcDeviceID() << "...\n";
  logger->log (oss.str());
  bcastMsgFromDevices.push(block);
  lock_bcastMsg.unlock();
  msgQueueCheck.notify_one();
}

void AdHocNetworkEmulator::getNeighborOfDevice(Device* devicePtr, std::vector<Device*>& list) const
{
  std::unordered_map<std::string, Device*>::const_iterator iter = deviceList.begin();
  while (iter != deviceList.end())
  {
    Device* theOtherDevicePtr = iter->second;
    if (devicePtr->getDeviceID() != theOtherDevicePtr->getDeviceID()) 
    {
//      if (getDistance(devicePtr, theOtherDevicePtr) <= COMMUNICATION_RANGE)
      if (getDistance(devicePtr, theOtherDevicePtr) <= getCommunicationRange(devicePtr->getPowerLevel()))
      {
        list.push_back(theOtherDevicePtr);
      } 
    }
    iter++;
  } 
}

double  AdHocNetworkEmulator::getCommunicationRange (unsigned int power) const
{
  return sqrt(power)/1000.0;
}

Device* AdHocNetworkEmulator::getDeviceByID(std::string id) const
{
  std::unordered_map<std::string, Device*>::const_iterator iter = deviceList.find(id);
  while (iter == deviceList.end())
    iter = deviceList.find(id); // awkard solution: just wait the device is created.
  assert (iter != deviceList.end());
  return iter->second;
} 
