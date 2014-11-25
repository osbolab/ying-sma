#include <sma/ccn/devicewithgps.hpp>

#include <sma/ccn/datablock.hpp>

#include <sma/ccn/controllayer.hpp>
#include <sma/ccn/contentdescriptor.hpp>

#include <sma/json.hpp>

#include <sma/actor.hpp>
#include <sma/context.hpp>
#include <sma/async.hpp>
#include <sma/message.hpp>
#include <sma/messenger.hpp>
#include <sma/gpscomponent.hpp>

#include <cstdlib>

#include <iostream>
#include <iomanip>

#include <cstring>
#include <string>
#include <sstream>

#include <chrono>
#include <ctime>

#include <vector>

#include <sys/types.h>
#include <sys/stat.h>


using namespace std::literals::chrono_literals;


int DeviceWithGPS::HEARTBEAT_INTERVAL = 10;
int DeviceWithGPS::DIRECTORY_SYNC_INTERVAL = 15;

DeviceWithGPS::DeviceWithGPS(sma::Context* ctx)
  : sma::Actor(ctx)
  , controlPlane(ctx, this, std::to_string(ctx->this_node()->id()))
{
  for (std::size_t message_type = 0; message_type < 5; ++message_type)
    subscribe(static_cast<sma::Message::Type>(message_type));

  async(std::bind(&DeviceWithGPS::processNeighborQuery, this)).do_in(10s);
  if (std::uint32_t(this_node()->id()) == 0) {
    log.i("-------------------------- session --------------------------");
    async(std::bind(&DeviceWithGPS::publishContent, this, "pic.jpg", "pic.jpg"))
        .do_in(2s);
    log.d("publishing pic.jpg in 2 seconds");
  }

  // Start the background notification cycles
  schedule_beacon(3);
  schedule_dir_broadcast(3);
}

void DeviceWithGPS::dispose() { disposed = true; }

DeviceWithGPS::~DeviceWithGPS() { log.t(__func__); }

void DeviceWithGPS::schedule_beacon(std::size_t seconds)
{
  if (!disposed) {
    int interval = std::rand() % (seconds / 2)
                   + seconds / 2;
    async(std::bind(&DeviceWithGPS::beaconing, this))
        .do_in(std::chrono::seconds(interval));
    beacon_scheduled = true;
  }
}
void DeviceWithGPS::beaconing()
{
  beacon_scheduled = false;
  if (disposed)
    return;

  // broadcast GPS
  DataBlock block(SMA::GPSBCAST);
  std::string gpsBroadCastData = getJsonGPS();
  if (gpsBroadCastData.empty()) {
    log.f("GPS is not available; cancelling beaconing");
    return;
  }
  int size = gpsBroadCastData.size() + 1;
  char* payload = new char[size];
  gpsBroadCastData.copy(payload, size - 1, 0);
  payload[size - 1] = '\0';
  block.createData(this_node(), payload, size);
  sendSignal(block);
  delete[] payload;

  schedule_beacon(HEARTBEAT_INTERVAL);
}

void DeviceWithGPS::schedule_dir_broadcast(std::size_t seconds)
{
  if (!disposed) {
    int interval = std::rand() % (seconds / 2)
                   + seconds / 2;
    async(std::bind(&DeviceWithGPS::broadcastDirectory, this))
        .do_in(std::chrono::seconds(interval));
    broadcast_scheduled = true;
  }
}
void DeviceWithGPS::broadcastDirectory()
{
  broadcast_scheduled = false;
  if (disposed)
    return;

  // broadcast content directory
  int numOfEntries = 5;    // Temp solution: The number should be provided by
                           // the profiling module.
  DataBlock block(SMA::DIRBCAST);
  std::string dirBroadCastData = getJsonDirectory(numOfEntries);
  int size = dirBroadCastData.size() + 1;
  char payload[size];
  dirBroadCastData.copy(payload, size - 1, 0);
  payload[size - 1] = '\0';
  block.createData(this_node(), payload, size);
  sendSignal(block);

  schedule_dir_broadcast(DIRECTORY_SYNC_INTERVAL);
}

void DeviceWithGPS::forwardRequest(std::string chunk)
{
  DataBlock block(SMA::REQUESTFWD);
  std::string requestFwdData = getJsonFwd(chunk);
  int size = requestFwdData.size() + 1;
  char* payload = new char[size];
  requestFwdData.copy(payload, size - 1, 0);
  payload[size - 1] = '\0';
  block.createData(this_node(), payload, size);
  sendSignal(block);
  delete[] payload;
  payload = nullptr;
  std::ostringstream logStr;
}

void DeviceWithGPS::sendSignal(const DataBlock& block)
{
  assert(block.payloadSize > 0);

  // narrowing
  auto dp = reinterpret_cast<const std::uint8_t*>(block.dataArray);
  auto msg = sma::Message::wrap(static_cast<sma::Message::Type>(block.dataType),
                                sma::Message::LIGHT,
                                dp,
                                block.payloadSize);
  post(msg);
}

void DeviceWithGPS::receive(sma::Message const& msg)
{
  DataBlock data(static_cast<SMA::MESSAGE_TYPE>(msg.type()));
  auto csrc = reinterpret_cast<const char*>(msg.cdata());
  data.createData(this_node(), csrc, msg.size());
  receiveSignal(data);
}
void DeviceWithGPS::receive(sma::Message const& msg, sma::Actor* sender)
{
  log.f("DeviceWithGPS::receive(Message const&, Actor*) is not implemented!");
}
void DeviceWithGPS::receiveSignal(const DataBlock& block)
{
  if (!controlPlane.processSignal(block))
    log.e("<-- invalid data block");
}

void DeviceWithGPS::processNeighborQuery() const
{
  std::vector<std::string> ids;
  controlPlane.getNeighborIDs(ids);
  std::stringstream buf;
  buf << "Neighbors (" << ids.size() << ") {";
  log.i("neighbors (%v) {", ids.size());
  for (auto it = ids.begin(); it != ids.end(); ++it)
    log.i("  %v,", controlPlane.getNeighborInfo(*it));
  log.i("}");
}

void DeviceWithGPS::publishContent(std::string inFileName,
                                   std::string outFileName)
{
  std::vector<std::pair<ContentAttribute::META_TYPE, std::string>> attri_pair;
  std::chrono::system_clock::time_point timestamp
      = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(timestamp);
  std::tm* gmtm = std::gmtime(&t);
  std::ostringstream oss;
  char tmc[30];
  std::strftime(tmc, 30, "%Y/%m/%d %T", gmtm);
  oss << tmc;
  log.i("publishing %v as %v", inFileName, outFileName);
  attri_pair.push_back(
      std::make_pair(ContentAttribute::PublishTime, oss.str()));
  controlPlane.publishContent(inFileName, outFileName, attri_pair);
}

void DeviceWithGPS::retrieveContentAs(std::string fileName,
                                      std::string exportFileName)
{
  //  controlPlane.restoreContentAs(fileName, exportFileName);  //for test
  controlPlane.retrieveContentAndSaveAs(fileName, exportFileName);
}

std::string DeviceWithGPS::getJsonGPS() const
{
  auto gps = context()->try_get_component<sma::GpsComponent>();
  if (!gps)
    return "";

  auto pos = gps->position();

  Json::Value result;
  Json::Value gpsobj;
  gpsobj["latitude"] = pos.lon;
  gpsobj["longitude"] = pos.lat;
  result["gps"] = gpsobj;
  result["device_id"] = std::string(this_node()->id());
  Json::StyledWriter styledWriter;    // should change fastWriter
  std::string jsonMessage = styledWriter.write(result);
  return jsonMessage;
}

std::string DeviceWithGPS::getJsonFwd(std::string chunk) const
{
  //  Json::Value result;
  Json::Value fwd;
  fwd["chunk_id"] = chunk;
  //  result.append(fwd);
  Json::StyledWriter styledWriter;
  //  std::string jsonMessage = styledWriter.write(result);
  std::string jsonMessage = styledWriter.write(fwd);
  return jsonMessage;
}

std::string DeviceWithGPS::getJsonDirectory(int numOfEntries) const
{
  Json::Value result;
  std::vector<ContentDescriptor> directory
      = controlPlane.getContentDirectory(numOfEntries);
  std::vector<ContentDescriptor>::iterator iter = directory.begin();
  while (iter != directory.end()) {
    Json::Value file;
    file["name"] = iter->getContentName();
    // add chunk list
    Json::Value chunks;
    std::vector<std::string> chunkList = iter->getChunkList();
    for (int i = 0; i < chunkList.size(); i++) {
      std::ostringstream istr;
      istr << i;
      chunks[istr.str()] = chunkList[i];
    }
    file["chunk_list"] = chunks;
    // add attribute list
    Json::Value metas;
    std::map<ContentAttribute::META_TYPE, std::string> metaPairs
        = iter->getMetaPairList();
    std::map<ContentAttribute::META_TYPE, std::string>::iterator attri_iter
        = metaPairs.begin();
    while (attri_iter != metaPairs.end()) {
      ContentAttribute::META_TYPE enumValue = attri_iter->first;
      metas[ContentAttribute::META_TYPE_STR[enumValue]] = attri_iter->second;
      file["attr_list"] = metas;
      attri_iter++;
    }
    iter++;
    // append one entry
    result.append(file);
  }
  Json::StyledWriter styledWriter;
  std::string jsonMessage = styledWriter.write(result);
  return jsonMessage;
}

void DeviceWithGPS::showDirectory() const { controlPlane.showDirectory(); }

void DeviceWithGPS::showPendingFiles() const
{
  controlPlane.showPendingFiles();
}

void DeviceWithGPS::showPendingChunksOfFile(std::string fileName) const
{
  controlPlane.showPendingChunksOfFile(fileName);
}
