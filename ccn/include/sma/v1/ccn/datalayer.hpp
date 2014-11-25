#pragma once

#include <sma/ccn/flowtable.hpp>
#include <sma/ccn/chunkstore.hpp>
#include <sma/ccn/pendingchunkmanager.hpp>

#include <sma/io/log>

#include <string>
#include <fstream>
#include <unordered_map>
#include <set>

class ControlLayer;
class DeviceLogger;

class DataLayer
{

public:
  DataLayer(sma::Logger log, ControlLayer* cl, std::string cacheDirName);
  ~DataLayer();
  void storeChunk(std::string fileName, bool requestedBySelf);
  bool hasChunk (std::string chunkID) const;
  void deleteChunk (std::string chunkID) const;
  void fetchChunk (std::string chunkID, std::ifstream& fin) const;
  void prepareChunks (std::string readFileName, std::vector<std::string> chunkList);
  void addFlowRule(std::string chunk, int rule);
  void delFlowRule(std::string chunk);
  int getFlowRule(std::string chunk) const;

  void showPendingChunksOfFile (std::string fileName) const;

private:
  sma::Logger log;
  ControlLayer* control;
  static std::string DEFAULT_CACHE_PREFIX;
  PendingChunkManager pendingChunkMgr;
  ChunkStore* store;
  FlowTable flowTable;
};



