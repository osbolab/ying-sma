#include <iostream>
#include <sma/ccn/networkemulator.hpp>
#include <sma/ccn/device.hpp>
#include <sma/ccn/adhocnetworkemulator.hpp>
#include <sma/ccn/devicewithgps.hpp>

using namespace std;

int main (int argc, char* argv[])
{
  AdHocNetworkEmulator* network = new AdHocNetworkEmulator();
  DeviceWithGPS* mobileA = new DeviceWithGPS("device-A");
  mobileA->setGPS(50.0358, -5.4253);
  DeviceWithGPS* mobileB = new DeviceWithGPS("device-B");
  mobileB->setGPS(50.0358, -5.42531);
  DeviceWithGPS* mobileC = new DeviceWithGPS("device-c");
  mobileC->setGPS(40, -30);
  mobileA->connectToNetwork(network);
  mobileB->connectToNetwork(network);
  mobileC->connectToNetwork(network);
//  network->showConnectedDevices();
  while(true) {}
//  delete network;
//  delete mobileA;
//  delete mobileB; 
}
