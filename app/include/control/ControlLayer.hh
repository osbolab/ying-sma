#ifndef CONTROL_LAYER_H
#define CONTROL_LAYER_H

#include "data/DataLayer.hh"
#include "control/Segmenter.hh"
#include "control/ContentDirectory.hh"
#include <string>
#include "control/ContentDiscription.hh"
#include <vector>
#include <unordered_map>
#include <mutex>
#include "node/Device.hh"
#include "control/NeighborRecords.hh"
#include "network/DataBlock.hh"
#include "control/SignalHandler.hh"
#include "control/PendingFileManager.hh"
#include "TypeDefinition.hh"

class ControlLayer
{
public:
  ControlLayer(std::string cacheNameInDatalayer);
  void publishContent(std::string inFileName, std::string outFileName, const std::vector<std::pair<ContentAttribute::META_TYPE, std::string> >& fileMeta);
  std::vector<ContentDiscription> getContentDirectory (int numOfEntries) const;  
  void updateDirectory(ContentDiscription discription);

  void showDirectory() const; //for test use
  void showPendingFiles() const;
  void showPendingChunksOfFile(std::string fileName) const;

  void notifyDownloadCompleted(std::string fileName);
  void retrieveContentAndSaveAs(std::string readFileName, std::string saveFileName);
  void addFlowRule(ChunkID chunk, int rule);
  void removeFlowRule(ChunkID);
  int getFlowRule(ChunkID) const;

  void forwardRequest(ChunkID chunk);

  void setDevicePtr (Device* devicePtr);

  void updateNeighborRecord(std::string id, double latitude, double longitude);
  std::string getNeighborInfo(std::string id) const;
  void getNeighborIDs(std::vector<std::string>& list) const;
  void transmitChunk (ChunkID chunk);
  void storeChunk (char* buffer, int sizeOfBuffer, bool requestedBySelf, ChunkID chunkID); //currently, checkMD5 = false, should be true.

  bool processSignal (const DataBlock& block);
  bool hasChunk (ChunkID chunk) const;
  void restoreContentAs(std::string readFileName, std::string saveFileName); 

  void addRuleToFlowTable (ChunkID chunk, int rule); 
  void delRuleFromFlowTable (ChunkID chunk);
  int getRuleFromFlowTable (ChunkID) const;

private:
  static std::string TMP_FOLDER;
  DataLayer datalayer;
  Segmenter segmenter;
  ContentDirectory directory;
  PendingFileManager pendingFileTable;
  Device* device;
  NeighborRecords neighborManager;
  SignalHandler signalHandler;
};

#endif
