#include <sma/app/emulator.hpp>

#include <sma/app/adhocnetworkemulator.hpp>
#include <sma/app/devicewithgps.hpp>
#include <sma/app/device.hpp>
#include <sma/app/batchscriptgenerator.hpp>

#include <iostream>
#include <sstream>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <cstdlib>
#include <locale>
#include <utility>

Emulator::Emulator()
  : network(nullptr)
  , running(true)
{
  //  initEnv();
}

void Emulator::runInBatch()
{
  BatchScriptGenerator newBatch;
  std::ifstream fin;
  newBatch.getScriptFin(fin);
  std::string line;
  while (running && std::getline(fin, line)) {
    processCommand(line);
  }
}

void Emulator::runInRealtime()
{
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "Please enter command >>>  ";
  std::string line;
  while (running && std::getline(std::cin, line)) {
    processCommand(line);
    std::cout << "Please enter command >>>  ";
  }
  std::cout << std::endl;
}

void Emulator::initEnv(bool inBatchMode = false)
{

  if (inBatchMode) {
    runInBatch();
    std::cout << "Enter any key to stop the emulation..." << std::endl;
    std::getchar();    // enter any key to stop.
  } else {
    runInRealtime();
  }
}

void Emulator::processCommand(std::string cmdLine)
{
  assert(!cmdLine.empty());
  std::vector<std::string> argv;
  std::istringstream iss(cmdLine);
  std::string subCmd;
  while (iss >> subCmd) {
    argv.push_back(subCmd);
  }

  std::transform(argv[0].begin(), argv[0].end(), argv[0].begin(), ::tolower);
  if (argv[0] == "create") {
    assert(argv.size() >= 3);
    std::transform(argv[1].begin(), argv[1].end(), argv[1].begin(), ::tolower);
    if (argv[1] == "network") {
      std::transform(
          argv[2].begin(), argv[2].end(), argv[2].begin(), ::tolower);
      assert(argv[2] == "adhoc");
      std::cout << "Creating network..." << std::endl;
      network = new AdHocNetworkEmulator();
    } else if (argv[1] == "device") {
      std::string deviceID = argv[2];
      if (network == nullptr) {
        std::cout
            << "Error when create devices: no network available to connect. "
            << std::endl;
        return;
      }
      std::cout << "Creating device " << deviceID << "..." << std::endl;
      // create device before the network is created.
      // currently, by default, only GPS-enabled devices can be created.
      DeviceWithGPS* mobileDevice = new DeviceWithGPS(deviceID);
      //      mobileDevice->connectToNetwork(network);
      devices.insert(make_pair(
          deviceID, mobileDevice));    // keep track of the devices created.
    } else {
      std::cout << "Invalid Command: only network or device can be created."
                << std::endl;
    }
  } else if (argv[0] == "setgps") {
    assert(argv.size() == 4);
    DeviceWithGPS* deviceHandler =
        (DeviceWithGPS*) (this->getDeviceByID(argv[1]));
    if (deviceHandler != nullptr) {
      double lat = std::atof(argv[2].c_str());
      double lon = atof(argv[3].c_str());
      deviceHandler->setGPS(lat, lon);
    }
  } else if (argv[0] == "setpower") {
    assert(argv.size() == 3);
    DeviceWithGPS* deviceHandler =
        (DeviceWithGPS*) (this->getDeviceByID(argv[1]));
    if (deviceHandler != nullptr) {
      int powerValue = std::atoi(argv[2].c_str());
      deviceHandler->setPowerLevel(powerValue);
    }
  } else if (argv[0] == "connect") {
    assert(argv.size() == 2);
    DeviceWithGPS* deviceHandler =
        (DeviceWithGPS*) (this->getDeviceByID(argv[1]));
    if (deviceHandler != nullptr) {
      deviceHandler->connectToNetwork(network);
    }
  } else if (argv[0] == "neighbor") {
    assert(argv.size() == 2);
    DeviceWithGPS* deviceHandler =
        (DeviceWithGPS*) (this->getDeviceByID(argv[1]));
    if (deviceHandler != nullptr) {
      deviceHandler->processNeighborQuery();
    }
  } else if (argv[0] == "directory") {
    assert(argv.size() == 2);
    DeviceWithGPS* deviceHandler =
        (DeviceWithGPS*) (this->getDeviceByID(argv[1]));
    if (deviceHandler != nullptr) {
      deviceHandler->showDirectory();
    }
  } else if (argv[0] == "publish") {
    assert(argv.size() == 4);
    DeviceWithGPS* deviceHandler =
        (DeviceWithGPS*) (this->getDeviceByID(argv[1]));
    if (deviceHandler != nullptr) {
      deviceHandler->publishContent(argv[2].c_str(), argv[3].c_str());
    }
  } else if (argv[0] == "retrieve") {
    assert(argv.size() == 4);
    DeviceWithGPS* deviceHandler =
        (DeviceWithGPS*) (this->getDeviceByID(argv[1]));
    if (deviceHandler != nullptr) {
      deviceHandler->retrieveContentAs(argv[2].c_str(), argv[3].c_str());
    }
  } else if (argv[0] == "quit") {
    clearObject();
    running = false;
  } else {
    std::cout << "Unknown command!" << std::endl;
  }
}

Device* Emulator::getDeviceByID(std::string id) const
{
  std::unordered_map<std::string, Device*>::const_iterator iter =
      devices.find(id);
  if (iter == devices.end()) {
    std::cout << "No device with Id " << id << std::endl;
    return nullptr;
  } else
    return iter->second;
}

void Emulator::clearObject()
{
  if (network != nullptr) {
    delete network;
    network = nullptr;
  }

  std::unordered_map<std::string, Device*>::const_iterator iter =
      devices.begin();
  while (iter != devices.end()) {
    Device* device = iter->second;
    delete device;
    device = nullptr;
    iter++;
  }
}
