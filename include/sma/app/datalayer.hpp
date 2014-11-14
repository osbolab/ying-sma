#pragma once


#include <string>
#include <sma/app/chunkstore.hpp>
#include <fstream>
#include <sma/app/typedefinition.hpp>
#include <unordered_map>
#include <set>
#include <sma/app/pendingchunkmanager.hpp>
#include <sma/app/flowtable.hpp>

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
  void prepareChunks (std::string readFileName, std::vector<ChunkID> chunkList);  
  void setControlLayer (ControlLayer* controlPtr);
  void addFlowRule(ChunkID chunk, int rule);
  void delFlowRule(ChunkID chunk);
  int getFlowRule(ChunkID chunk) const;
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


