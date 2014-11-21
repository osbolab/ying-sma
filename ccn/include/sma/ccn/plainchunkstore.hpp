#pragma once


#include <sma/ccn/chunkstore.hpp>
#include <unordered_map>
#include <mutex>
#include <string>
#include <fstream>


class PlainChunkStore : public ChunkStore
{
public:
  PlainChunkStore(std::string dirName);
  bool hasChunk (std::string chunkID) const;
  std::string storeChunk (std::string tmpChunkFile);
  void deleteChunk (std::string chunkID);
  void fetchChunk (std::string chunkID, std::ifstream& fin) const;
  void setLogger (DeviceLogger* loggerPtr);
  ~PlainChunkStore();

private:
  PlainChunkStore(const PlainChunkStore& ); // put the operator= in the private region, too
  std::string cacheDir;
  void saveToFS(std::string chunkID);
  void delFromFS(std::string chunkID);
  std::unordered_map <std::string, std::string> m_chunkIndex;
  std::mutex m_mutex;
  DeviceLogger* logger;
//  static std::string DEFAULT_DIR;
};



