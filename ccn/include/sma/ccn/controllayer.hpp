#pragma once


#include <sma/ccn/datalayer.hpp>
#include <sma/ccn/segmenter.hpp>
#include <sma/ccn/contentdirectory.hpp>
#include <sma/ccn/contentdescriptor.hpp>
#include <sma/ccn/device.hpp>
#include <sma/ccn/neighborrecords.hpp>
#include <sma/ccn/datablock.hpp>
#include <sma/ccn/signalhandler.hpp>
#include <sma/ccn/pendingfilemanager.hpp>

#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>

class ControlLayer
{
public:
  ControlLayer(std::string cacheNameInDatalayer);
  void publishContent(std::string inFileName, std::string outFileName, const std::vector<std::pair<ContentAttribute::META_TYPE, std::string> >& fileMeta);
  std::vector<ContentDescriptor> getContentDirectory (int numOfEntries) const;
  void updateDirectory(ContentDescriptor descriptor);

  void showDirectory() const; //for test use
  void showPendingFiles() const;
  void showPendingChunksOfFile(std::string fileName) const;

  void notifyDownloadCompleted(std::string fileName);
  void retrieveContentAndSaveAs(std::string readFileName, std::string saveFileName);
  void addFlowRule(std::string chunk, int rule);
  void removeFlowRule(std::string);
  int getFlowRule(std::string) const;

  void forwardRequest(std::string chunk);

  void setDevicePtr (Device* devicePtr);

  void updateNeighborRecord(std::string id, double latitude, double longitude);
  std::string getNeighborInfo(std::string id) const;
  void getNeighborIDs(std::vector<std::string>& list) const;
  void transmitChunk (std::string chunk);
  void storeChunk (char* buffer, int sizeOfBuffer, bool requestedBySelf, std::string chunkID); //currently, checkMD5 = false, should be true.

  bool processSignal (const DataBlock& block);
  bool hasChunk (std::string chunk) const;
  void restoreContentAs(std::string readFileName, std::string saveFileName);

  void addRuleToFlowTable (std::string chunk, int rule);
  void delRuleFromFlowTable (std::string chunk);
  int getRuleFromFlowTable (std::string) const;

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


