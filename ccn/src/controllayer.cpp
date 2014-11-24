#include <sma/ccn/controllayer.hpp>
#include <sma/ccn/contentdescriptor.hpp>
#include <sma/ccn/signalhandler.hpp>
#include <sma/ccn/contentdirectory.hpp>
#include <sma/ccn/segmenter.hpp>
#include <sma/ccn/devicewithgps.hpp>

#include <sma/context.hpp>
#include <sma/io/log>

#include <string>
#include <mutex>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

std::string ControlLayer::TMP_FOLDER = "tmp/";

ControlLayer::ControlLayer(sma::Context* ctx, DeviceWithGPS* dev, std::string cacheNameInDatalayer)
  : ctx(ctx)
  , dev(dev)
  , signalHandler(ctx->log(), this)
  , datalayer(ctx->log(), this, cacheNameInDatalayer)
{
}

void ControlLayer::publishContent(
    std::string inFileName,
    std::string outFileName,
    const std::vector<std::pair<ContentAttribute::META_TYPE, std::string>>&
        fileMeta)
{
  std::vector<std::string> chunkIDs;
  segmenter.storeFile(inFileName, chunkIDs, datalayer);
  ContentDescriptor newFile(outFileName);
  std::vector<std::string>::iterator chunk_iter = chunkIDs.begin();
  int id = 0;
  while (chunk_iter != chunkIDs.end()) {
    newFile.addNewChunk(id, *chunk_iter);
    id++;
    chunk_iter++;
  }

  std::vector<std::pair<ContentAttribute::META_TYPE,
                        std::string>>::const_iterator attri_iter
      = fileMeta.begin();
  while (attri_iter != fileMeta.end()) {
    newFile.addAttribute(attri_iter->first, attri_iter->second);
    attri_iter++;
  }

  updateDirectory(newFile);
}

void ControlLayer::updateDirectory(ContentDescriptor description)
{
  directory.addContentToDirectory(description);
}

bool ControlLayer::processSignal(const DataBlock& block)
{
  return signalHandler.processSignal(block);
}


void ControlLayer::retrieveContentAndSaveAs(std::string readFileName,
                                            std::string saveFileName)
{
  LOG(DEBUG) << "retrieve " << readFileName << " into " << saveFileName;

  std::vector<std::string> chunkList = directory.getChunkList(readFileName);
  if (chunkList.size() > 0) {
    pendingFileTable.addTask(readFileName, saveFileName);
    datalayer.prepareChunks(readFileName, chunkList);
  } else {
    LOG(WARNING) << "No chunk record available!";
  }
}


void ControlLayer::notifyDownloadCompleted(std::string fileName)
{
  std::string saveFileName = pendingFileTable.getExportName(fileName);
  if (!saveFileName.empty()) {
    this->restoreContentAs(fileName, saveFileName);
    pendingFileTable.delTask(fileName);
  }
}


void ControlLayer::restoreContentAs(std::string readFileName,
                                    std::string saveFileName)
{
  LOG(DEBUG) << "rebuilding file: " << readFileName;
  std::vector<std::string> chunkList = directory.getChunkList(readFileName);
  if (chunkList.size() > 0) {
    segmenter.loadFileFromChunks(
        saveFileName,
        chunkList,
        datalayer);    // should change to a callback mechanism
  } else {
    LOG(ERROR) << "no chunk record available to restore file";
  }
}

void ControlLayer::showDirectory() const
{
  std::vector<ContentDescriptor> serviceDirectory = directory.getDirectory();
  std::vector<ContentDescriptor>::iterator iter = serviceDirectory.begin();
  while (iter != serviceDirectory.end()) {
    iter->print();
    iter++;
  }
}

void ControlLayer::showPendingFiles() const { pendingFileTable.print(); }

void ControlLayer::showPendingChunksOfFile(std::string fileName) const
{
  datalayer.showPendingChunksOfFile(fileName);
}

std::vector<ContentDescriptor>
ControlLayer::getContentDirectory(int numOfEntries) const
{
  return directory.getNDirectory(numOfEntries);
}

void ControlLayer::addFlowRule(std::string chunk, int rule)
{
  datalayer.addFlowRule(chunk, rule);
}

void ControlLayer::removeFlowRule(std::string chunk)
{
  datalayer.delFlowRule(chunk);
}

int ControlLayer::getFlowRule(std::string chunk) const
{
  return datalayer.getFlowRule(chunk);
}


void ControlLayer::forwardRequest(std::string chunk)
{
  dev->forwardRequest(chunk);
}

void ControlLayer::updateNeighborRecord(std::string id,
                                        double latitude,
                                        double longitude)
{
  neighborManager.updateRecord(id, latitude, longitude);
}

std::string ControlLayer::getNeighborInfo(std::string id) const
{
  return neighborManager.getNeighborInfo(id);
}

void ControlLayer::getNeighborIDs(std::vector<std::string>& list) const
{
  neighborManager.getNeighborIDs(list);
}

void ControlLayer::transmitChunk(std::string chunk)
{
  if (this->hasChunk(chunk)) {
    std::ifstream fin;
    datalayer.fetchChunk(chunk, fin);
    if (fin.is_open()) {
      int sizeOfChunk = fin.tellg();
      fin.seekg(0, std::ios::beg);
      char* buffer = new char[sizeOfChunk];
      fin.read(buffer, sizeOfChunk);
      fin.close();
      DataBlock block(SMA::CHUNK);
      block.setChunkID(chunk);
      block.createData(ctx->this_node(), buffer, sizeOfChunk);
      delete[] buffer;
      buffer = nullptr;
      dev->sendSignal(block);
    }
  }
}

bool ControlLayer::hasChunk(std::string chunk) const
{
  return datalayer.hasChunk(chunk);
}

void ControlLayer::storeChunk(char* buffer,
                              int sizeOfBuffer,
                              bool requestedBySelf,
                              std::string chunkID)
{
  std::ostringstream oss;
  oss << TMP_FOLDER << chunkID;
  std::ofstream tmpOut(oss.str());
  tmpOut.write(buffer, sizeOfBuffer);
  tmpOut.close();
  datalayer.storeChunk(oss.str(), requestedBySelf);
  if (!chunkID.empty()) {
    std::remove(oss.str().c_str());
  }
}

void ControlLayer::addRuleToFlowTable(std::string chunk, int rule)
{
  datalayer.addFlowRule(chunk, rule);
}

void ControlLayer::delRuleFromFlowTable(std::string chunk)
{
  datalayer.delFlowRule(chunk);
}

int ControlLayer::getRuleFromFlowTable(std::string chunk) const
{
  return datalayer.getFlowRule(chunk);
}
