#include <sma/ccn/datalayer.hpp>
#include <sma/ccn/plainchunkstore.hpp>
#include <sma/ccn/pendingchunkmanager.hpp>

#include <sma/ccn/segmenter.hpp>
#include <sma/ccn/controllayer.hpp>

#include <sma/io/log>

#include <string>
#include <sys/stat.h>
#include <vector>
#include <sstream>

#include <exception>

#include <sys/types.h>
#include <sys/stat.h>


std::string DataLayer::DEFAULT_CACHE_PREFIX = "cache/";

DataLayer::DataLayer(sma::Logger log,
                     ControlLayer* cl,
                     std::string cacheDirName)
  : log(std::move(log))
  , control(cl)
{
  struct stat info;
  if (stat(DEFAULT_CACHE_PREFIX.c_str(), &info) == -1) {
    if (mkdir(DEFAULT_CACHE_PREFIX.c_str(),
              S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
      log.f("can't make cache directory '%v'", DEFAULT_CACHE_PREFIX);
      throw std::exception();
    } else
      log.d("created cache directory '%v'", DEFAULT_CACHE_PREFIX);
  } else if (!(info.st_mode & S_IFDIR)) {
    log.f("cache path '%v' isn't a directory", DEFAULT_CACHE_PREFIX);
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
  std::string chunkID = store->storeChunk(fileName);
  // update pendingChunkMgr
  if (requestedBySelf) {
    std::vector<std::string> fileCompleted;
    pendingChunkMgr.completeDownloadTask(chunkID, fileCompleted);
    for (auto it = fileCompleted.begin(); it != fileCompleted.end(); ++it) {
      log.d("file complete: %v", *it);
      control->notifyDownloadCompleted(*it);
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

void DataLayer::prepareChunks(std::string fileName,
                              std::vector<std::string> chunkList)
{
  bool chunkAllAvailable = true;
  for (auto it = chunkList.begin(); it != chunkList.end(); ++it) {
    if (this->hasChunk(*it))
      continue;
    // for file requested by the application layer
    pendingChunkMgr.addDownloadTask(fileName, *it);
    flowTable.addRule(*it, 0);
    control->forwardRequest(*it);
    chunkAllAvailable = false;
    // requested by self;
    this->addFlowRule(*it, 0);
  }
  if (chunkAllAvailable)
    control->notifyDownloadCompleted(fileName);
}

void DataLayer::addFlowRule(std::string chunk, int rule)
{
  flowTable.addRule(chunk, rule);
}

void DataLayer::delFlowRule(std::string chunk) { flowTable.delRule(chunk); }

int DataLayer::getFlowRule(std::string chunk) const
{
  return flowTable.getRule(chunk);
}

void DataLayer::showPendingChunksOfFile(std::string fileName) const
{
  pendingChunkMgr.printRemainingChunksOfFile(fileName);
}
