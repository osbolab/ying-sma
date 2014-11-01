#include "devicewithgps.hh"
#include "devicelogger.hh"

#include "datablock.hh"

#include "controllayer.hh"
#include "contentdiscription.hh"

#include "json.hh"

#include <iostream>
#include <iomanip>

#include <string>
#include <sstream>

#include <chrono>
#include <ctime>

#include <vector>

#include <thread>


int DeviceWithGPS::HEARTBEAT_INTERVAL = 10;
int DeviceWithGPS::DIRECTORY_SYNC_INTERVAL = 30;
std::string DeviceWithGPS::LOG_DIR = "./log/";
unsigned int DeviceWithGPS::DEFAULT_POWER_LEVEL = 10000;

DeviceWithGPS::DeviceWithGPS(std::string id)
  : deviceID(id)
  , network(nullptr)
  , controlPlane(id)
{
  gpsDriver.setGPS(0.0, 0.0);
  powerLevel = DEFAULT_POWER_LEVEL;
  std::string logFileName = LOG_DIR + id + ".log";
  logger = new DeviceLogger(logFileName);
  controlPlane.setDevicePtr(this);
}

DeviceWithGPS::~DeviceWithGPS()
{
  logger->close();
  delete logger;
  logger = nullptr;
  beaconingThread.detach();    // these two signals can be move to SignalHandler
  directorySyncThread.detach();
}

std::string DeviceWithGPS::getDeviceID() const
{
  return deviceID;
}

bool DeviceWithGPS::hasGPS()
{
  return gpsDriver.hasGPS();
}

void DeviceWithGPS::setGPS(double latitude, double longitude)
{
  gpsDriver.setGPS(latitude, longitude);
}

void DeviceWithGPS::setPowerLevel(unsigned int level)
{
  powerLevel = level;
}

unsigned int DeviceWithGPS::getPowerLevel() const
{
  return powerLevel;
}

GPSinfo DeviceWithGPS::getGPS() const
{
  return gpsDriver.getGPS();
}

/* Services will be launched once the device is connected to the network
 */
void DeviceWithGPS::connectToNetwork(NetworkEmulator* networkToAttach)
{
  if (network == nullptr && networkToAttach) {
    logger->log("Connected.\n");
    networkToAttach->acceptDevice(this);
    network = networkToAttach;
    beaconingThread = std::thread(&DeviceWithGPS::beaconing, this);
    directorySyncThread = std::thread(&DeviceWithGPS::broadcastDirectory, this);
  }
}

void DeviceWithGPS::leaveFromNetwork(NetworkEmulator* networkAttached)
{
  if (networkAttached)
    networkAttached->dropDevice(this);
}

/* Currently, the beconning message broadcast the GPS information
 * together with the user ID to the network.
 */
void DeviceWithGPS::beaconing()
{
  // broadcast GPS
  while (true) {
    if (network == nullptr)
      break;
    DataBlock block(SMA::GPSBCAST);
    std::string gpsBroadCastData =
        getJsonGPS();    // create Json-formatted beaconing message
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
    std::this_thread::sleep_for(std::chrono::seconds(HEARTBEAT_INTERVAL));
  }
}

void DeviceWithGPS::broadcastDirectory()
{
  // broadcast content directory
  int numOfEntries = 5;    // Temp solution: The number should be provided by
                           // the profiling module.
  while (true) {
    if (network == nullptr)
      break;
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
    std::this_thread::sleep_for(std::chrono::seconds(DIRECTORY_SYNC_INTERVAL));
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
  //  if (block.getMsgType() == "CHUNK")
  //    std::cout << "In DeviceWithGPS::sendSignal: "<< '\n'
  //              << "The chunk id is " << block.getChunkID() << '\n';
  network->receiveMsg(block);
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
  std::chrono::system_clock::time_point timestamp =
      std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(timestamp);
  std::tm* gmtm = std::gmtime(&t);
  std::ostringstream oss;
  char tmc[30];
  std::strftime(tmc, 30, "%Y/%m/%d %T", gmtm);
  oss << tmc;
  oss.put(0);
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
  std::vector<ContentDiscription> directory =
      controlPlane.getContentDirectory(numOfEntries);
  std::vector<ContentDiscription>::iterator iter = directory.begin();
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
    std::map<ContentAttribute::META_TYPE, std::string> metaPairs =
        iter->getMetaPairList();
    std::map<ContentAttribute::META_TYPE, std::string>::iterator attri_iter =
        metaPairs.begin();
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

void DeviceWithGPS::showDirectory() const
{
  controlPlane.showDirectory();
}

void DeviceWithGPS::showPendingFiles() const
{
  controlPlane.showPendingFiles();
}

void DeviceWithGPS::showPendingChunksOfFile(std::string fileName) const
{
  controlPlane.showPendingChunksOfFile(fileName);
}

DeviceLogger* DeviceWithGPS::getLoggerPtr() const
{
  return logger;
}
