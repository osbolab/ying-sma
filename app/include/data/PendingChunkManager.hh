#ifndef PENDING_CHUNK_MANAGER_H
#define PENDING_CHUNK_MANAGER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <mutex>
#include "TypeDefinition.h"

class PendingChunkManager
{
public:
  void addDownloadTask(std::string fileName, ChunkID chunk);
  void completeDownloadTask(ChunkID chunk, std::vector<std::string> &filesCompleted);
  void printRemainingChunksOfFile(std::string fileName) const;

private:
  void removeChunkFromFile (ChunkID chunk, std::string fileName, bool &complete);
  std::unordered_map<std::string, std::set<ChunkID>* > pendingChunksInFile;
  std::unordered_map<ChunkID, std::set<std::string>* > pendingFilesOfChunk;
  std::mutex m_mutex_file;
  std::mutex m_mutex_chunk;
};

#endif
