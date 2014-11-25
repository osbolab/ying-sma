#pragma once

#include <string>
#include <fstream>

class ChunkStore
{
public:
  virtual bool hasChunk (std::string chunkID) const = 0;
  virtual std::string storeChunk (std::string tmpChunkFile) = 0; // May throw exception
  virtual void deleteChunk (std::string chunkID) = 0;
  virtual void fetchChunk (std::string chunkID, std::ifstream& fin) const = 0;
  virtual ~ChunkStore(){};
};


