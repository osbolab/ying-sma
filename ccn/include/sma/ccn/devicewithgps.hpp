#pragma once

#include <sma/actor.hpp>
#include <sma/context.hpp>

#include <sma/ccn/device.hpp>

#include <sma/ccn/datablock.hpp>
#include <sma/ccn/neighborrecords.hpp>
#include <sma/ccn/controllayer.hpp>
#include <sma/ccn/devicelogger.hpp>

#include <mutex>
#include <atomic>
#include <queue>


namespace sma
{
struct Message;
}

class DeviceWithGPS : public sma::Actor, public Device
{

public:
  DeviceWithGPS(sma::Context* ctx);
  ~DeviceWithGPS();

  void dispose();

  virtual std::string getDeviceID() const override;

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
  virtual void receive(sma::Message const& msg) override;
  virtual void receive(sma::Message const& msg, Actor* sender) override;

  virtual void receiveSignal(DataBlock const& block) override;
  virtual void sendSignal(DataBlock const& block) override;
  /* for test use
 * commands come from the terminals
 */
  void processNeighborQuery() const;
  virtual DeviceLogger* getLoggerPtr() const override;


private:
  std::atomic_bool disposed{false};
  std::atomic_bool broadcast_scheduled{false};
  std::atomic_bool beacon_scheduled{false};
  static std::string LOG_DIR;
  std::string deviceID;
  std::queue<DataBlock> inputQueue;    // listening to the network
  std::mutex m_mutex_queue_i;

  static int HEARTBEAT_INTERVAL;
  static int DIRECTORY_SYNC_INTERVAL;
  static unsigned int DEFAULT_POWER_LEVEL;

  /* task delegation
 */
  ControlLayer controlPlane;
  DeviceLogger* logger;


  void beaconing();
  virtual void forwardRequest(std::string chunk) override;
  void broadcastDirectory();
  std::string
  getJsonGPS() const;    // format the gps data returned from GPS driver.
  std::string getJsonDirectory(int numOfEntries) const;
  std::string getJsonFwd(std::string chunk) const;
  void processBeaconing(DataBlock const& block);
  void processDirectorySync(DataBlock const& block);
};
