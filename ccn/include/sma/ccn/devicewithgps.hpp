#pragma once

#include <sma/actor.hpp>
#include <sma/context.hpp>

#include <sma/ccn/datablock.hpp>
#include <sma/ccn/neighborrecords.hpp>
#include <sma/ccn/controllayer.hpp>

#include <mutex>
#include <atomic>
#include <queue>


namespace sma
{
struct Message;
}

class DeviceWithGPS : public sma::Actor
{
public:
  DeviceWithGPS(sma::Context* ctx);
  ~DeviceWithGPS();

  // sma::Actor
  virtual void dispose() override;
  // Callback from sma::messenger (from NS3)
  // Unpacks the contents as a DataBlock and gives it to receiveSignal(1)
  virtual void receive(sma::Message const& msg) override;
  virtual void receive(sma::Message const& msg, Actor* sender) override;


  void publishContent(std::string inFileName, std::string outFileName);
  void retrieveContentAs(std::string fileName, std::string exportFileName);
  void forwardRequest(std::string chunk);

  void showDirectory() const;
  void showPendingFiles() const;
  void showPendingChunksOfFile(std::string fileName) const;

  void receiveSignal(DataBlock const& block);
  void sendSignal(DataBlock const& block);
  /* for test use
 * commands come from the terminals
 */
  void processNeighborQuery() const;


private:
  // We may run into the situation where the scheduler has tasks queued for us
  // when we get deconstructed. To avoid blowing it up, for now we sleep on
  // our heartbeat interval until both xx_scheduled are false.
  // FIXME: Implement unscheduling so we can do it properly.
  std::atomic_bool disposed{false};
  std::atomic_bool broadcast_scheduled{false};
  std::atomic_bool beacon_scheduled{false};

  static int HEARTBEAT_INTERVAL;
  static int DIRECTORY_SYNC_INTERVAL;
  static unsigned int DEFAULT_POWER_LEVEL;

  ControlLayer controlPlane;

  void beaconing();
  void broadcastDirectory();
  std::string getJsonGPS() const;
  std::string getJsonDirectory(int numOfEntries) const;
  std::string getJsonFwd(std::string chunk) const;
  void processBeaconing(DataBlock const& block);
  void processDirectorySync(DataBlock const& block);
};
