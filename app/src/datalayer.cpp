#include <sma/app/datalayer.hpp>
#include <sma/app/plainchunkstore.hpp>
#include <sma/app/pendingchunkmanager.hpp>

#include <sma/app/devicelogger.hpp>

#include <sma/app/segmenter.hpp>
#include <sma/app/controllayer.hpp>

#include <sma/log>

#include <string>
#include <sys/stat.h>
#include <vector>
#include <sstream>

#include <exception>

#include <sys/types.h>
#include <sys/stat.h>


std::string DataLayer::DEFAULT_CACHE_PREFIX = "cache/";

DataLayer::DataLayer(std::string cacheDirName)
{
  struct stat info;
  if (stat(DEFAULT_CACHE_PREFIX.c_str(), &info) == -1) {
    if (mkdir(DEFAULT_CACHE_PREFIX.c_str(),
              S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
      LOG(ERROR) << "Can't make directory " << DEFAULT_CACHE_PREFIX;
      throw std::exception();
    } else
      LOG(DEBUG) << "Created log directory " << DEFAULT_CACHE_PREFIX;
  } else if (!(info.st_mode & S_IFDIR)) {
    LOG(ERROR) << DEFAULT_CACHE_PREFIX << " isn't a directory";
    throw std::exception();
  }


  std::string completeCachePath = DataLayer::DEFAULT_CACHE_PREFIX + cacheDirName
                                  + "/";
  mkdir(completeCachePath.c_str(), 0777);
  store = new PlainChunkStore(completeCachePath);
}

DataLayer::~DataLayer()
{
  delete store;
  store = nullptr;
}

void DataLayer::storeChunk(std::string fileName, bool requestedBySelf)
{
  ChunkID chunkID = store->storeChunk(fileName);
  // update pendingChunkMgr
  if (requestedBySelf) {
    std::vector<std::string> fileCompleted;
    pendingChunkMgr.completeDownloadTask(chunkID, fileCompleted);
    std::vector<std::string>::iterator iter = fileCompleted.begin();
    while (iter != fileCompleted.end()) {
      std::ostringstream oss;
      oss << "file completed: " << *iter << '\n';
      logger->log(oss.str());
      control->notifyDownloadCompleted(*iter);
      iter++;
    }
  }
}

bool DataLayer::hasChunk(std::string chunkID) const
{
  return store->hasChunk(chunkID);
}

void DataLayer::deleteChunk(std::string chunkID) const
{
  store->deleteChunk(chunkID);
}

void DataLayer::fetchChunk(std::string chunkID, std::ifstream& fin) const
{
  store->fetchChunk(chunkID, fin);
}

void DataLayer::setControlLayer(ControlLayer* controlPtr)
{
  control = controlPtr;
}


void DataLayer::prepareChunks(std::string fileName,
                              std::vector<ChunkID> chunkList)
{
  bool chunkAllAvailable = true;
  std::vector<ChunkID>::iterator iter = chunkList.begin();
  while (iter != chunkList.end()) {
    if (!this->hasChunk(*iter)) {
      pendingChunkMgr.addDownloadTask(
          fileName, *iter);    // for file requested by the application layer
      flowTable.addRule(*iter, 0);
      control->forwardRequest(*iter);
      chunkAllAvailable = false;
      this->addFlowRule(*iter, 0);    // requested by self;
    }
    iter++;
  }
  if (chunkAllAvailable)
    control->notifyDownloadCompleted(
        fileName);    // the call back function to notify the control layer
}

void DataLayer::addFlowRule(ChunkID chunk, int rule)
{
  flowTable.addRule(chunk, rule);
}

void DataLayer::delFlowRule(ChunkID chunk) { flowTable.delRule(chunk); }

int DataLayer::getFlowRule(ChunkID chunk) const
{
  return flowTable.getRule(chunk);
}

void DataLayer::showPendingChunksOfFile(std::string fileName) const
{
  pendingChunkMgr.printRemainingChunksOfFile(fileName);
}

void DataLayer::setLogger(DeviceLogger* loggerPtr) { logger = loggerPtr; }
