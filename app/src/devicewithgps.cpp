#include <sma/app/devicewithgps.hpp>
#include <sma/app/devicelogger.hpp>

#include <sma/app/datablock.hpp>

#include <sma/app/controllayer.hpp>
#include <sma/app/contentdescriptor.hpp>

#include <sma/json.hpp>

#include <sma/context.hpp>
#include <sma/message.hpp>
#include <sma/messenger.hpp>
#include <sma/async>
#include <sma/log>

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



int DeviceWithGPS::HEARTBEAT_INTERVAL = 10;
int DeviceWithGPS::DIRECTORY_SYNC_INTERVAL = 30;
std::string DeviceWithGPS::LOG_DIR = "logs/nodes/";

DeviceWithGPS::DeviceWithGPS(sma::context ctx)
  : actor(std::move(ctx))
  , network(nullptr)
  , controlPlane("0")
  , ctx(ctx)
{
  deviceID = "0";
  for (std::size_t message_type = 0; message_type < 5; ++message_type) {
    subscribe(static_cast<sma::message_type>(message_type));
  }
  gpsDriver.setGPS(0.0, 0.0);

  struct stat info;
  if (stat(LOG_DIR.c_str(), &info) == -1) {
    if (mkdir(LOG_DIR.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
      LOG(ERROR) << "Can't make directory " << LOG_DIR;
    else
      LOG(DEBUG) << "Created log directory " << LOG_DIR;
  } else if (!(info.st_mode & S_IFDIR))
    LOG(ERROR) << LOG_DIR << " isn't a directory";
  std::string logFileName = LOG_DIR + deviceID + ".log";
  logger = new DeviceLogger(logFileName);
  controlPlane.setDevicePtr(this);
  // Start the background notification cycles
  beaconing();
  broadcastDirectory();
}

void DeviceWithGPS::dispose() { disposed = true; }

DeviceWithGPS::~DeviceWithGPS()
{
  while (beacon_scheduled || broadcast_scheduled)
    std::this_thread::sleep_for(std::chrono::seconds(HEARTBEAT_INTERVAL));
  logger->close();
  delete logger;
  logger = nullptr;
}

std::string DeviceWithGPS::getDeviceID() const { return deviceID; }

/* Currently, the beconning message broadcast the GPS information
 * together with the user ID to the network.
 */
void DeviceWithGPS::beaconing()
{
  beacon_scheduled = false;
  if (disposed)
    return;

  // broadcast GPS
  DataBlock block(SMA::GPSBCAST);
  std::string gpsBroadCastData
      = getJsonGPS();    // create Json-formatted beaconing message
  int size = gpsBroadCastData.size() + 1;
  char* payload = new char[size];
  gpsBroadCastData.copy(payload, size - 1, 0);
  payload[size - 1] = '\0';
  block.createData(this, payload, size);
  sendSignal(block);
  delete[] payload;

  std::ostringstream logStr;
  logStr << "Heartbeating...\n";
  logger->log(logStr.str());

  if (!disposed) {
    sma::async(std::bind(&DeviceWithGPS::beaconing, this))
        .do_in(std::chrono::seconds(HEARTBEAT_INTERVAL));
    beacon_scheduled = true;
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
  char* payload = new char[size];
  dirBroadCastData.copy(payload, size - 1, 0);
  payload[size - 1] = '\0';
  block.createData(this, payload, size);
  sendSignal(block);
  delete[] payload;

  std::ostringstream logStr;
  logStr << "Broadcast directory sync to the network...\n";
  logger->log(logStr.str());

  if (!disposed) {
    sma::async(std::bind(&DeviceWithGPS::broadcastDirectory, this))
        .do_in(std::chrono::seconds(DIRECTORY_SYNC_INTERVAL));
    broadcast_scheduled = true;
  }
}

void DeviceWithGPS::forwardRequest(ChunkID chunk)
{
  if (network != nullptr) {
    DataBlock block(SMA::REQUESTFWD);
    std::string requestFwdData = getJsonFwd(chunk);
    int size = requestFwdData.size() + 1;
    char* payload = new char[size];
    requestFwdData.copy(payload, size - 1, 0);
    payload[size - 1] = '\0';
    block.createData(this, payload, size);
    sendSignal(block);
    delete[] payload;
    payload = nullptr;
    std::ostringstream logStr;
    logStr << deviceID << ": Request Forwarding for " << chunk << '\n';
    logger->log(logStr.str());
  }
}

void DeviceWithGPS::sendSignal(const DataBlock& block)
{
  assert(block.payloadSize > 0);

  // narrowing
  auto dp = reinterpret_cast<const std::uint8_t*>(block.dataArray);
  sma::message m(static_cast<sma::message_type>(block.dataType),
                 std::move(dp),
                 block.payloadSize);
  post(m);
}

void DeviceWithGPS::on_message(const sma::message& msg)
{
  DataBlock data(static_cast<SMA::MESSAGE_TYPE>(msg.type()));
  auto csrc = reinterpret_cast<const char*>(msg.cdata());
  data.createData(this, csrc, msg.size());
  receiveSignal(data);
}
void DeviceWithGPS::receiveSignal(const DataBlock& block)
{
  //  char* payload = new char [block.getPayloadSize()];
  //  block.getPayload(payload);
  //  logFile << "The payload is: " << payload << '\n';
  //  lockLog.unlock();
  if (controlPlane.processSignal(block) == false)    // fail to process signal
  {
    logger->log("Error: Invalid Signal.\n");
  }
}

void DeviceWithGPS::processNeighborQuery() const
{
  std::vector<std::string> ids;
  controlPlane.getNeighborIDs(ids);
  std::vector<std::string>::iterator iter = ids.begin();
  std::cout << "The neighbor records of " << this->getDeviceID()
            << " is:" << std::endl;
  while (iter != ids.end()) {
    std::cout << controlPlane.getNeighborInfo(*iter) << std::endl;
    iter++;
  }
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
  Json::Value result;
  Json::Value gps;
  GPSinfo gpsData = gpsDriver.getGPS();
  gps["latitude"] = gpsData.latitude;
  gps["longitude"] = gpsData.longitude;
  result["gps"] = gps;
  result["device_id"] = this->getDeviceID();
  Json::StyledWriter styledWriter;    // should change fastWriter
  std::string jsonMessage = styledWriter.write(result);
  return jsonMessage;
}

std::string DeviceWithGPS::getJsonFwd(ChunkID chunk) const
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

DeviceLogger* DeviceWithGPS::getLoggerPtr() const { return logger; }
