#pragma once


#include <sma/app/gpsinfo.hpp>
#include <sma/app/device.hpp>
#include <sma/app/networkemulator.hpp>

#include <sma/app/context.hpp>

#include <sma/app/datablock.hpp>
#include <mutex>
#include <atomic>
#include <queue>
#include <sma/app/neighborrecords.hpp>
#include <sma/app/controllayer.hpp>
#include <sma/app/gpsdriver.hpp>
#include <sma/app/devicelogger.hpp>
#include <sma/app/typedefinition.hpp>


class DeviceWithGPS : public Device
{

public:
  DeviceWithGPS(sma::context ctx);
  ~DeviceWithGPS();

  void dispose();

  std::string getDeviceID() const;
  /* GPS driver delegation
 */
  bool hasGPS();
  GPSinfo getGPS() const;
  void setGPS(double latitude, double longitude); // should be a private method later, reflected by the user mobility.


  void setPowerLevel (unsigned int level);
  unsigned int getPowerLevel () const;

  void connectToNetwork(NetworkEmulator* networkToAttach);
  void leaveFromNetwork(NetworkEmulator* networkAttached);
  void publishContent(std::string inFileName, std::string outFileName);
  void retrieveContentAs(std::string fileName, std::string exportFileName);

  void showDirectory() const;
  void showPendingFiles() const;
  void showPendingChunksOfFile(std::string fileName) const;
  /*
 * interface exposed to the network to receive broadcast message, e.g., NS3.
 * cuirrently, the data transmitted is a user-defined network. It should be
 * changed to the byte array when connecting to the real network.
 */
  // Callback from sma::messenger (from NS3)
  // Unpacks the contents as a DataBlock and gives it to receiveSignal(1)
  void on_message(const sma::message& msg);

  void receiveSignal(const DataBlock& block);
  void sendSignal(const DataBlock& block); //interface to deliver data to the network.
  /* for test use
 * commands come from the terminals
 */
  void processNeighborQuery () const;
  DeviceLogger* getLoggerPtr() const;


private:
  std::atomic_bool disposed{false};
  std::atomic_bool broadcast_scheduled{false};
  std::atomic_bool beacon_scheduled{false};
  static std::string LOG_DIR;
  NetworkEmulator* network;
  sma::context ctx;
  std::string deviceID;
  std::queue<DataBlock> inputQueue; //listening to the network
  std::mutex m_mutex_queue_i;
  unsigned int powerLevel;

  static int HEARTBEAT_INTERVAL;
  static int DIRECTORY_SYNC_INTERVAL;
  static unsigned int DEFAULT_POWER_LEVEL;

  /* task delegation
 */
  ControlLayer controlPlane;
  GPSDriver gpsDriver;
  DeviceLogger* logger;


  void beaconing();
  void forwardRequest(ChunkID chunk);
  void broadcastDirectory();
  std::string getJsonGPS() const;   //format the gps data returned from GPS driver.
  std::string getJsonDirectory(int numOfEntries) const;
  std::string getJsonFwd(ChunkID chunk) const;
  void processBeaconing (const DataBlock& block);
  void processDirectorySync (const DataBlock& block);


};


