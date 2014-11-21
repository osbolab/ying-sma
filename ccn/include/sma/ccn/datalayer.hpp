#pragma once

#include <sma/ccn/flowtable.hpp>
#include <sma/ccn/chunkstore.hpp>
#include <sma/ccn/pendingchunkmanager.hpp>

#include <string>
#include <fstream>
#include <unordered_map>
#include <set>

class ControlLayer;
class DeviceLogger;

class DataLayer
{

public:
  DataLayer(std::string cacheDirName);
  ~DataLayer();
  void storeChunk(std::string fileName, bool requestedBySelf);
  bool hasChunk (std::string chunkID) const;
  void deleteChunk (std::string chunkID) const;
  void fetchChunk (std::string chunkID, std::ifstream& fin) const;
  void prepareChunks (std::string readFileName, std::vector<std::string> chunkList);
  void setControlLayer (ControlLayer* controlPtr);
  void addFlowRule(std::string chunk, int rule);
  void delFlowRule(std::string chunk);
  int getFlowRule(std::string chunk) const;
  void setLogger(DeviceLogger* loggerPtr);

  void showPendingChunksOfFile (std::string fileName) const;

private:
  ChunkStore* store;
  ControlLayer* control;
  DeviceLogger* logger;
  static std::string DEFAULT_CACHE_PREFIX;
  PendingChunkManager pendingChunkMgr;
  FlowTable flowTable;
};


