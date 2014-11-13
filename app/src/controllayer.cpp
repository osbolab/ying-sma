#include <sma/app/controllayer.hpp>
#include <sma/app/contentdiscription.hpp>
#include <string>
#include <sma/app/contentdirectory.hpp>
#include <sma/app/segmenter.hpp>
#include <vector>
#include <iostream>
#include <mutex>
#include <sma/app/signalhandler.hpp>
#include <fstream>
#include <sstream>
#include <cstdio>

std::string ControlLayer::TMP_FOLDER = "./build/tmp/";

ControlLayer::ControlLayer(std::string cacheNameInDatalayer):datalayer(cacheNameInDatalayer)
{
    datalayer.setControlLayer(this);
    signalHandler.setControlLayer(this);
}

void ControlLayer::setDevicePtr (Device* devicePtr)
{
  device = devicePtr;
  signalHandler.setLogger(device->getLoggerPtr());
  datalayer.setLogger(device->getLoggerPtr());
  segmenter.setLogger(device->getLoggerPtr());
}

void ControlLayer::publishContent(std::string inFileName, std::string outFileName, const std::vector<std::pair<ContentAttribute::META_TYPE, std::string> >& fileMeta)
{
  std::vector<ChunkID> chunkIDs;
  segmenter.storeFile(inFileName, chunkIDs, datalayer);
  ContentDiscription newFile(outFileName);
  std::vector<ChunkID>::iterator chunk_iter = chunkIDs.begin();
  int id = 0;
  while (chunk_iter != chunkIDs.end())
  {
    newFile.addNewChunk(id, *chunk_iter);
    id++;
    chunk_iter++;
  }

  std::vector<std::pair<ContentAttribute::META_TYPE, std::string> >::const_iterator attri_iter = fileMeta.begin();
  while (attri_iter != fileMeta.end()) 
  {
    newFile.addAttribute(attri_iter->first, attri_iter->second);
    attri_iter++;
  }

  updateDirectory(newFile);
}

void ControlLayer::updateDirectory(ContentDiscription discription)
{
  directory.addContentToDirectory(discription);
}

bool ControlLayer::processSignal (const DataBlock& block)
{
  return signalHandler.processSignal(block);
}


void ControlLayer::retrieveContentAndSaveAs(std::string readFileName, std::string saveFileName)
{
  std::ostringstream oss;
  oss << "Retrieving content " << readFileName << " which will be saved to " << saveFileName << std::endl;
  device->getLoggerPtr()->log (oss.str());
  std::vector<ChunkID> chunkList = directory.getChunkList(readFileName);
  if (chunkList.size() > 0)
  {
    pendingFileTable.addTask(readFileName, saveFileName);
    datalayer.prepareChunks(readFileName, chunkList);
  }
  else
  {
    device->getLoggerPtr()->log ("No chunk record available!\n");
  }
}


void ControlLayer::notifyDownloadCompleted(std::string fileName)
{
  std::string saveFileName = pendingFileTable.getExportName(fileName);
  if (!saveFileName.empty())
  {
    this->restoreContentAs(fileName, saveFileName);
    pendingFileTable.delTask(fileName); 
  }
}


void ControlLayer::restoreContentAs(std::string readFileName, std::string saveFileName)
{
  std::ostringstream oss; 
  oss << "Restoring the file: " << readFileName << std::endl;
  device->getLoggerPtr()->log (oss.str());
  std::vector<ChunkID> chunkList = directory.getChunkList(readFileName);
  if (chunkList.size() > 0)
  {
    segmenter.loadFileFromChunks (saveFileName, chunkList, datalayer); //should change to a callback mechanism
  }
  else
  {
    device->getLoggerPtr()->log ("No chunk record available when restoring the file...\n");
  }
}

void ControlLayer::showDirectory() const
{
  std::vector<ContentDiscription> serviceDirectory = directory.getDirectory();
  std::vector<ContentDiscription>::iterator iter = serviceDirectory.begin();
  while (iter != serviceDirectory.end())
  {
    iter->print();
    iter++;
  }
}

void ControlLayer::showPendingFiles() const
{
  pendingFileTable.print();
}

void ControlLayer::showPendingChunksOfFile (std::string fileName) const
{
  datalayer.showPendingChunksOfFile (fileName);
}

std::vector<ContentDiscription> ControlLayer::getContentDirectory (int numOfEntries) const
{
  return directory.getNDirectory(numOfEntries); 
}

void ControlLayer::addFlowRule (ChunkID chunk, int rule)
{
  datalayer.addFlowRule(chunk, rule);
}

void ControlLayer::removeFlowRule (ChunkID chunk)
{
  datalayer.delFlowRule(chunk);
}

int ControlLayer::getFlowRule (ChunkID chunk) const
{
  return datalayer.getFlowRule(chunk);
}


void ControlLayer::forwardRequest (ChunkID chunk)
{
  device->forwardRequest (chunk); //for future revision: request should be properly scheduled.
}

void ControlLayer::updateNeighborRecord(std::string id, double latitude, double longitude)
{
  neighborManager.updateRecord(id, latitude, longitude); 
}

std::string ControlLayer::getNeighborInfo(std::string id) const
{
  return neighborManager.getNeighborInfo (id);
}

void ControlLayer::getNeighborIDs(std::vector<std::string>& list) const
{
  neighborManager.getNeighborIDs(list);
}

void ControlLayer::transmitChunk (ChunkID chunk)
{
  if (this->hasChunk(chunk))
  {
    std::ifstream fin;
    datalayer.fetchChunk (chunk, fin);
    if (fin.is_open())
    {
      int sizeOfChunk = fin.tellg();
      fin.seekg(0, std::ios::beg);
      char* buffer = new char [sizeOfChunk];
      fin.read (buffer, sizeOfChunk); 
      fin.close();
      DataBlock block (SMA::CHUNK);
      block.setChunkID (chunk);
      block.createData (device, buffer, sizeOfChunk);
      delete [] buffer;
      buffer = nullptr;
      device->sendSignal(block);
    }
  }
}

bool ControlLayer::hasChunk (ChunkID chunk) const
{
  return datalayer.hasChunk(chunk);
}

void ControlLayer::storeChunk (char* buffer, int sizeOfBuffer, bool requestedBySelf, ChunkID chunkID)
{
  std::ostringstream oss;
  oss << TMP_FOLDER << chunkID;
  std::ofstream tmpOut (oss.str()); 
  tmpOut.write (buffer, sizeOfBuffer);
  tmpOut.close();
  datalayer.storeChunk (oss.str(), requestedBySelf);
  if (!chunkID.empty())
  {
    std::remove(oss.str().c_str());  
  }
}

void ControlLayer::addRuleToFlowTable (ChunkID chunk, int rule)
{
  datalayer.addFlowRule (chunk, rule);
}

void ControlLayer::delRuleFromFlowTable (ChunkID chunk)
{
  datalayer.delFlowRule (chunk);
}

int ControlLayer::getRuleFromFlowTable (ChunkID chunk) const
{
  return datalayer.getFlowRule (chunk);
}
