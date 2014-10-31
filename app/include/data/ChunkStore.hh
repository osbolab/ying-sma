#ifndef CHUNK_STORE_H
#define CHUNK_STORE_H

#include <string>
#include <fstream>
#include "TypeDefinition.h"
#include "DeviceLogger.h"

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

#endif
