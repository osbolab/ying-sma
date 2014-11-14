#pragma once


#include <string>
#include <fstream>
#include <sma/app/typedefinition.hpp>
#include <sma/app/devicelogger.hpp>

class ChunkStore
{
public:
  virtual bool hasChunk (std::string chunkID) const = 0;   
  virtual ChunkID storeChunk (std::string tmpChunkFile) = 0; // May throw exception
  virtual void deleteChunk (std::string chunkID) = 0;
  virtual void fetchChunk (std::string chunkID, std::ifstream& fin) const = 0;
  virtual void setLogger (DeviceLogger* loggerPtr) = 0;
  virtual ~ChunkStore(){};
};


